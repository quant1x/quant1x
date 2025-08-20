import os
import random

from ray import tune
from ray.tune import TuneConfig
from ray.tune.search.hebo import HEBOSearch
from ray.air.config import RunConfig


def objective(params):
    loss = 1
    return {'val_loss': loss}


if __name__ == '__main__':
    root_path = os.getcwd()
    model_saved_path = os.path.join(root_path, 'saved_models')
    ray_tune_rslt_path = os.path.join(root_path, 'ray_tune_rslt')
    train_data_path = os.path.join(root_path, 'data')

    previously_run_params = [
    ]

    known_rewards = [-189, -1144]
    algo = HEBOSearch(
        metric='val_loss',
        mode='min',
        points_to_evaluate=previously_run_params,
        evaluated_rewards=known_rewards,
        random_state_seed=123,
        max_concurrent=1
    )

    param_space_cfg = {
        'root_path': root_path,
        'model_folder': model_saved_path,
        'num_epochs': 200,
        'early_stopping_patience': 10,
        'multiprocessing_workers': 5,
        'dropout_rate': tune.quniform(0.1, 0.9, 0.1),           #0.1
        'hidden_layer_size': tune.qrandint(128, 200, 4),        #160
        'learning_rate': tune.choice([1e-4, 1e-3, 1e-2, 1e-1]),
        'minibatch_size': tune.qrandint(16, 64, 4),
        'max_gradient_norm': tune.choice([0.01, 1.0, 100.0]),
        'num_heads': tune.randint(5, 30),
        'stack_size': tune.choice([1, 2, 3, 4]),
        'total_time_steps': tune.randint(60, 196),
        'num_multi_horizon': tune.choice([1, 2, 3, 4, 5, 6, 7]),
        'num_encoder_steps': 128,
    }

    tune_cfg = TuneConfig(
        metric="val_loss",
        mode="min",
        search_alg=algo,
        num_samples=500,
    )

    run_cfg = RunConfig(
        name='stock_tune_run',
        local_dir=ray_tune_rslt_path
    )

    tuner = tune.Tuner(
        objective,
        tune_config=tune_cfg,
        run_config=run_cfg,
        param_space=param_space_cfg
    )

    rslt = tuner.fit()

    print(rslt.get_best_result().config)

