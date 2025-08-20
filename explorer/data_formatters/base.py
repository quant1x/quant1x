import abc
import enum


class DataTypes(enum.IntEnum):
    REAL_VALUED = 0
    CATEGORICAL = 1
    DATE = 2


class InputTypes(enum.IntEnum):
    TARGET = 0
    OBSERVED_INPUT = 1
    KNOWN_INPUT = 2
    STATIC_INPUT = 3
    ID = 4
    TIME = 5


class GenericFormatter(abc.ABC):
    @abc.abstractmethod
    def set_scalers(self, df):
        raise NotImplementedError()

    @abc.abstractmethod
    def transform_inputs(self, df):
        raise NotImplementedError()

    @abc.abstractmethod
    def split_data(self, df):
        raise NotImplementedError()

    @property
    @abc.abstractmethod
    def _column_definition(self):
        raise  NotImplementedError()

    @abc.abstractmethod
    def get_fixed_params(self):
        raise NotImplementedError

    @property
    def num_classes_per_cat_input(self):
        return self._num_classes_per_cat_input

    def get_num_samples_for_calibration(self):
        return -1, -1

    def get_column_definition(self):
        column_definition = self._column_definition

        def _check_single_column(input_type):
            length = len([tup for tup in column_definition if tup[2] == input_type])

            if length != 1:
                raise ValueError('Illegal number of inputs ({}) of type {}'.format(
                                    length, input_type))
        _check_single_column(InputTypes.ID)
        _check_single_column(InputTypes.TIME)

        identifier = [tup for tup in column_definition if tup[2] == InputTypes.ID]
        time = [tup for tup in column_definition if tup[2] == InputTypes.TIME]
        real_inputs = [
            tup for tup in column_definition if tup[1] == DataTypes.REAL_VALUED and
                                                tup[2] not in {InputTypes.ID, InputTypes.TIME}
        ]
        categorical_inputs = [
            tup for tup in column_definition if tup[1] == DataTypes.CATEGORICAL and
                                                tup[2] not in {InputTypes.ID, InputTypes.TIME}
        ]

        return identifier + time + real_inputs + categorical_inputs

    def _get_input_columns(self):
        return [
            tup[0]
            for tup in self.get_column_definition()
            if tup[2] not in {InputTypes.ID, InputTypes.TIME}
        ]

    def _get_tft_input_indices(self):
        def _extract_tuples_from_data_type(data_type, defn):
            return [
                tup for tup in defn if tup[1] == data_type and
                                       tup[2] not in {InputTypes.ID, InputTypes.TIME}
            ]

        def _get_locations(input_types, defn):
            return [i for i, tup in enumerate(defn) if tup[2] in input_types]

        column_definition = [
            tup for tup in self.get_column_definition()
            if tup[2] not in {InputTypes.ID, InputTypes.TIME}
        ]

        categorical_inputs = _extract_tuples_from_data_type(DataTypes.CATEGORICAL,
                                                            column_definition)
        real_inputs = _extract_tuples_from_data_type(DataTypes.REAL_VALUED,
                                                     column_definition)

        locations = {
            'input_size':
                len(self._get_input_columns()),
            'output_size':
                len(_get_locations({InputTypes.TARGET}, column_definition)),
            'category_counts':
                self.num_classes_per_cat_input,
            'input_obs_loc':
                _get_locations({InputTypes.TARGET}, column_definition),
            'static_input_loc':
                _get_locations({InputTypes.STATIC_INPUT}, column_definition),
            'known_regular_inputs':
                _get_locations({InputTypes.STATIC_INPUT, InputTypes.KNOWN_INPUT},
                               real_inputs),
            'known_categorical_inputs':
                _get_locations({InputTypes.STATIC_INPUT, InputTypes.KNOWN_INPUT},
                               categorical_inputs),
        }

        return locations

    def get_experiment_params(self):
        required_keys = [
            'total_time_steps', 'num_encoder_steps', 'num_epochs',
            'early_stopping_patience', 'multiprocessing_workers'
        ]

        fixed_params = self.get_fixed_params()

        for k in required_keys:
            if k not in fixed_params:
                raise ValueError('Field {}'.format(k) +
                                 ' missing from fixed parameter definitions!')

        fixed_params['column_definition'] = self.get_column_definition()

        fixed_params.update(self._get_tft_input_indices())

        return fixed_params
