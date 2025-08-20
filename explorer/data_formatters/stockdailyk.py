import data_formatters.base
import libs.utils as utils
import pandas as pd
import sklearn.preprocessing

GeneratorDataFormatter = data_formatters.base.GenericFormatter
DataTypes = data_formatters.base.DataTypes
InputTypes = data_formatters.base.InputTypes

class StockDailyKFormatter(GeneratorDataFormatter):
    _column_definition = [
        ('Symbol', DataTypes.REAL_VALUED, InputTypes.ID),
        ('Name', DataTypes.CATEGORICAL, InputTypes.STATIC_INPUT),
        ('Date', DataTypes.REAL_VALUED, InputTypes.TIME),
        #('MarketCAPFloat', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        ('Close', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        ('High', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        ('Low', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        ('Change', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        #('CHGRate', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        ('TurnoverRatio', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        ('Turnover', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        ('Volume', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),

        #('AvgPrice', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        #('PrevClose', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        ('Amplitude', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        ('MarketCAP', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        #('SharedOutstanding', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        #('ShsFloat', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        ('PETTM', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        ('PB', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        ('PEStatic', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        #('BidAskPct', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        #('VolumePct', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        ('NetInflowVolume', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),
        ('NetInflowAmount', DataTypes.REAL_VALUED, InputTypes.OBSERVED_INPUT),

        ('days_from_start', DataTypes.REAL_VALUED, InputTypes.KNOWN_INPUT),
        ('day_of_week', DataTypes.CATEGORICAL, InputTypes.KNOWN_INPUT),
        ('day_of_month', DataTypes.CATEGORICAL, InputTypes.KNOWN_INPUT),
        ('week_of_year', DataTypes.CATEGORICAL, InputTypes.KNOWN_INPUT),
        ('month', DataTypes.CATEGORICAL, InputTypes.KNOWN_INPUT),
        ('Open', DataTypes.REAL_VALUED, InputTypes.TARGET),
    ]

    def __init__(self):
        self.identifiers = None
        self._real_scalers = None
        self._cat_scalers = None
        self._target_scalers = None
        self._num_classes_per_cat_input = None
        self._time_steps = self.get_fixed_params()['total_time_steps']

    def split_data(self, df, valid_boundary=128, test_boundary=256):
        print('Formatting train-valid-test splits.')

        fixed_params = self.get_fixed_params()
        example_nums = len(df)
        forecast_horizon = fixed_params['total_time_steps'] - fixed_params['num_encoder_steps']

        valid_steps = 8
        train = df.iloc[0 : example_nums - 2 * forecast_horizon - valid_steps]
        valid = df.iloc[example_nums - fixed_params['total_time_steps'] - forecast_horizon - valid_steps
                        : example_nums - forecast_horizon]
        test = df.iloc[example_nums - fixed_params['total_time_steps']
                       : example_nums]

        self.set_scalers(train)

        return (self.transform_inputs(data) for data in [train, valid, test])

    def set_scalers(self, df):
        print('Setting scalers with training data...')
        column_definitions = self.get_column_definition()
        id_column = utils.get_single_col_by_input_type(InputTypes.ID, column_definitions)
        target_column = utils.get_single_col_by_input_type(InputTypes.TARGET, column_definitions)

        real_inputs = utils.extract_cols_from_data_type(DataTypes.REAL_VALUED, column_definitions,
                                                        {InputTypes.ID, InputTypes.TIME})

        self._real_scalers = {}
        self._target_scalers = {}

        indentifiers = []
        for indentifier, sliced in df.groupby(id_column):
            if len(sliced) >= self._time_steps:
                data = sliced[real_inputs].values
                targets = sliced[[target_column]].values

                self._real_scalers[indentifier] = sklearn.preprocessing.StandardScaler().fit(data)
                self._target_scalers[indentifier] = sklearn.preprocessing.StandardScaler().fit(targets)

                indentifiers.append(indentifier)

        categorical_inputs = utils.extract_cols_from_data_type(
            DataTypes.CATEGORICAL, column_definitions,
            {InputTypes.ID, InputTypes.TIME}
        )

        categorical_scalers = {}
        num_classes = []

        for col in categorical_inputs:
            srs = df[col].apply(str)
            categorical_scalers[col] = sklearn.preprocessing.LabelEncoder().fit(srs.values)
            num_classes.append(srs.nunique())

        self._cat_scalers = categorical_scalers
        self._num_classes_per_cat_input = num_classes

        self.identifiers = indentifiers

    def transform_inputs(self, df):
        print('Transform inputs ...')
        if self._real_scalers is None and self._cat_scalers is None:
            raise ValueError('Scalers have not been set')

        column_definitions = self.get_column_definition()
        id_col = utils.get_single_col_by_input_type(InputTypes.ID, column_definitions)
        real_inputs = utils.extract_cols_from_data_type(
            DataTypes.REAL_VALUED, column_definitions,
            {InputTypes.ID, InputTypes.TIME}
        )
        categorical_inputs = utils.extract_cols_from_data_type(
            DataTypes.CATEGORICAL, column_definitions,
            {InputTypes.ID, InputTypes.TIME}
        )
        df_list = []
        for identifier, sliced in df.groupby(id_col):
            if len(sliced) >= self._time_steps:
                sliced_copy = sliced.copy()
                sliced_copy[real_inputs] = self._real_scalers[identifier].transform(
                                                            sliced_copy[real_inputs].values)
                df_list.append(sliced_copy)

        output = pd.concat(df_list, axis=0)

        for col in categorical_inputs:
            string_df = df[col].apply(str)
            output[col] = self._cat_scalers[col].transform(string_df)

        return output

    def format_predictions(self, predictions):
        if self._target_scalers is None:
            raise ValueError('Scalers have not been set!')

        column_names = predictions.columns

        df_list = []

        for identifier, sliced in predictions.groupby('identifier'):
            sliced_copy = sliced.copy()
            target_scaler = self._target_scalers[identifier]

            for col in column_names:
                if col not in {'forecast_time', 'identifier'}:
                    sliced_copy[col] = target_scaler.inverse_transform(sliced_copy[col].values.reshape(1, -1))
            df_list.append(sliced_copy)

        output = pd.concat(df_list, axis=0)

        return output

    def get_fixed_params(self):
        fixed_params = {
            'total_time_steps': 164,
            'num_encoder_steps': 156,
            'num_epochs': 200,
            'early_stopping_patience': 8,
            'multiprocessing_workers': 10
        }

        return fixed_params

    def get_default_model_params(self):
        model_params = {
            'dropout_rate': 0.03,
            'hidden_layer_size': 160,
            'learning_rate': 0.001,
            'minibatch_size': 32,
            'max_gradient_norm': 0.01,
            'num_heads': 8,
            'stack_size': 2
        }

        return model_params

    def get_num_samples_for_calibration(self):
        #return 450000, 50000
        #return 132 * 10, 132 * 5
        return -1, -1
