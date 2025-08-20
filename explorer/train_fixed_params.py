import argparse
import datetime as dte
import os

import data_formatters.base
import expt_settings.configs
import libs.hyperparam_opt
import libs.tft_model
import libs.utils as utils
import numpy as np
import pandas as pd
import tensorflow as tf

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'

ExperimentConfig = expt_settings.configs.ExperimentConfig
HyperparamOptManager = libs.hyperparam_opt.HyperparamOptManager
ModelClass = libs.tft_model.TemporalFusionTransformer

if __name__ == "__main__":
    root_folder = os.getcwd()
    stock_symbol = '002812'

    config = ExperimentConfig(stock_symbol, root_folder)
    data_formatter = config.make_data_formatter()

    data_csv_file = './data/' + stock_symbol + '_example.csv'
    tf_config = utils.get_default_tensorflow_config(tf_device='GPU', gpu_id=0)
    example_data = pd.read_csv(data_csv_file)
    example_data.fillna(method='ffill')
    train, valid, test = data_formatter.split_data(example_data)
    train_samples, valid_samples = data_formatter.get_num_samples_for_calibration()

    fixed_params = data_formatter.get_experiment_params()
    params = data_formatter.get_default_model_params()
    
    # hidden_layer_size
    params['model_folder'] = os.path.join(config.model_folder, 'fixed')

    print("*** Loading hyperparam manager ***")
    opt_manager = HyperparamOptManager({k: [params[k]] for k in params},
                                       fixed_params, config.model_folder)

    # Training -- one iteration only
    print("*** Running calibration ***")
    print("Params Selected:")
    for k in params:
        print("{}: {}".format(k, params[k]))

    use_gpu = True
    tf.compat.v1.experimental.output_all_intermediates(True)
    default_keras_session = tf.compat.v1.keras.backend.get_session()

    repeat_num = 10
    
    best_loss = np.Inf
    for repeat in range(repeat_num):
        tf.compat.v1.reset_default_graph()
        with tf.Graph().as_default(), tf.compat.v1.Session(config=tf_config) as sess:
            tf.compat.v1.keras.backend.set_session(sess)

            params = opt_manager.get_next_parameters()
            model = ModelClass(params, use_cudnn=use_gpu)

            if not model.training_data_cached():
                model.cache_batched_data(train, 'train', num_samples=train_samples)
                model.cache_batched_data(valid, 'valid', num_samples=valid_samples)

            sess.run(tf.compat.v1.global_variables_initializer())
            model.fit()

            val_loss = model.evaluate()
            print("Repeat {} val_loss {}" . format(repeat, val_loss))
            if val_loss < best_loss:
                opt_manager.update_score(params, val_loss, model)
                best_loss = val_loss

            tf.compat.v1.keras.backend.set_session(default_keras_session)

    print("*** Running tests ***")
    tf.compat.v1.reset_default_graph()
    with tf.compat.v1.Graph().as_default(), tf.compat.v1.Session(config=tf_config) as sess:
        tf.compat.v1.keras.backend.set_session(sess)
        best_params = opt_manager.get_best_params()
        model = ModelClass(best_params, use_cudnn=use_gpu)

        model.load(opt_manager.hyperparam_folder)
        print("Computing best validation loss")
        val_loss = model.evaluate(valid)

        print("Computing test loss")
        output_map = model.predict(test, return_targets=True)
        targets = data_formatter.format_predictions(output_map["targets"])
        p10_forecast = data_formatter.format_predictions(output_map["p10"])
        p50_forecast = data_formatter.format_predictions(output_map["p50"])
        p90_forecast = data_formatter.format_predictions(output_map["p90"])
    
        def extract_numerical_data(data):
            """Strips out forecast time and identifier columns."""
            return data[[
                col for col in data.columns
                if col not in {"forecast_time", "identifier"}
            ]]

        p10_loss = utils.numpy_normalised_quantile_loss(
                    extract_numerical_data(targets), extract_numerical_data(p10_forecast),
                    0.1)
        p50_loss = utils.numpy_normalised_quantile_loss(
            extract_numerical_data(targets), extract_numerical_data(p50_forecast),
            0.5)
        p90_loss = utils.numpy_normalised_quantile_loss(
            extract_numerical_data(targets), extract_numerical_data(p90_forecast),
            0.9)
        
        test_rslt = pd.concat(
            [extract_numerical_data(data) for data in [targets, p10_forecast, p50_forecast, p90_forecast]])
        
        params = data_formatter.get_fixed_params()
        multi_horizon = params['total_time_steps'] - params['num_encoder_steps']
        draw_steps = 20
        offset = len(example_data) - (draw_steps - 1) * multi_horizon - params['total_time_steps']
        dst = pd.DataFrame()
        for step in range(draw_steps - 1):
            offset += multi_horizon

            raw_input = example_data.iloc[offset:(offset + params['total_time_steps'])]
            output_map = model.predict(data_formatter.transform_inputs(raw_input), return_targets=True)
            #pre_targes.append()

            targets = data_formatter.format_predictions(output_map["targets"])

            p10_forecast = data_formatter.format_predictions(output_map["p10"])
            p50_forecast = data_formatter.format_predictions(output_map["p50"])
            p90_forecast = data_formatter.format_predictions(output_map["p90"])

            step_rslt = pd.concat([extract_numerical_data(data) for data in [targets, p10_forecast, p50_forecast, p90_forecast]])
            dst = pd.concat([dst, step_rslt], ignore_index=True, axis=1)

        dst = pd.concat([dst, test_rslt], ignore_index=True, axis=1)
        dst.to_csv('dst.csv')

        tf.compat.v1.keras.backend.set_session(default_keras_session)

        print("Training completed @ {}".format(dte.datetime.now()))
        print("Best validation loss = {}".format(val_loss))
        print("Params:")

        for k in best_params:
            print(k, " = ", best_params[k])
        print()
        print("Normalised Quantile Loss for Test Data: P10={}, P50={}, P90={}".format(
                    p10_loss.mean(), p50_loss.mean(), p90_loss.mean()))
