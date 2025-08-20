import os
import data_formatters.stockdailyk

default_experiments = ['stockdailyk']

class ExperimentConfig(object):
    def __init__(self, stock_symbol=None, root_folder=None):
        if root_folder is None:
            root_folder = os.path.join(
                os.path.dirname(os.path.relpath(__file__)), '..', 'outputs'
            )
            print('Usring root folder {}' . format(root_folder))

        self.root_folder = root_folder
        self.data_folder = os.path.join(root_folder, 'data')
        self.model_folder = os.path.join(root_folder, 'saved_models')
        self.result_folder = os.path.join(root_folder, 'results')

        for relevent_directory in [
            self.root_folder, self.data_folder, self.model_folder, self.result_folder
        ]:
            if not os.path.exists(relevent_directory):
                os.mkdir(relevent_directory)

    @property
    def data_csv_path(self):
        csv_map = {
        }

        return os.path.join(self.root_folder, 'data', '600243.csv')

    @property
    def hyperparam_iterations(self):
        return 200

    def make_data_formatter(self):
        return data_formatters.stockdailyk.StockDailyKFormatter()

