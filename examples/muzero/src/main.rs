use tch::{nn, Device, Kind, Tensor};
use rand::Rng;

// 1. 神经网络定义
#[derive(Debug)]
struct MuZeroNetwork {
    representation: nn::Sequential,
    dynamics: nn::Sequential,
    prediction: nn::Sequential,
    vs: nn::VarStore,
}

impl MuZeroNetwork {
    fn new() -> Self {
        let vs = nn::VarStore::new(Device::Cpu);
        let root = vs.root();

        let representation = nn::seq()
            .add(nn::linear(&root, 9, 16)) // 输入9格状态
            .add_fn(|x| x.relu());

        let dynamics = nn::seq()
            .add(nn::linear(&root, 16 + 1, 16)) // 动作编码为1维
            .add_fn(|x| x.relu());

        let prediction = nn::seq()
            .add(nn::linear(&root, 16, 16))
            .add_fn(|x| x.relu())
            .add(nn::linear(&root, 16, 2)); // 策略+价值

        Self {
            representation,
            dynamics,
            prediction,
            vs,
        }
    }

    fn initial_inference(&self, state: &Tensor) -> (Tensor, (Tensor, Tensor)) {
        let hidden = self.representation.forward(state);
        let pred = self.prediction.forward(&hidden);
        let (policy, value) = (pred.get(0), pred.get(1).sigmoid());
        (hidden, (policy, value))
    }

    fn recurrent_inference(&self, hidden: &Tensor, action: i64) -> (Tensor, (Tensor, Tensor), f32) {
        let action_tensor = Tensor::one_hot(1, 9, Kind::Float, Device::Cpu); // 简化动作编码
        let dynamics_in = Tensor::cat(&[hidden, &action_tensor], 1);
        let new_hidden = self.dynamics.forward(&dynamics_in);
        let pred = self.prediction.forward(&new_hidden);
        let (policy, value) = (pred.get(0), pred.get(1).sigmoid());
        (new_hidden, (policy, value), 0.0) // 暂不处理奖励
    }
}

// 2. 井字棋环境
#[derive(Clone, Copy, Debug)]
struct TicTacToe {
    board: [i8; 9],
    player: i8,
}

impl TicTacToe {
    fn new() -> Self {
        Self {
            board: [0; 9],
            player: 1,
        }
    }

    fn legal_actions(&self) -> Vec<usize> {
        self.board
            .iter()
            .enumerate()
            .filter(|(_, &v)| v == 0)
            .map(|(i, _)| i)
            .collect()
    }

    fn step(&mut self, action: usize) -> (f32, bool) {
        self.board[action] = self.player;
        let done = self.check_win();
        self.player *= -1;
        (if done { 1.0 } else { 0.0 }, done)
    }

    fn check_win(&self) -> bool {
        let wins = [
            [0, 1, 2], [3, 4, 5], [6, 7, 8], // 行
            [0, 3, 6], [1, 4, 7], [2, 5, 8], // 列
            [0, 4, 8], [2, 4, 6], // 对角线
        ];
        wins.iter().any(|&[a, b, c]| {
            self.board[a] != 0 && self.board[a] == self.board[b] && self.board[b] == self.board[c]
        })
    }

    fn to_tensor(&self) -> Tensor {
        Tensor::of_slice(&self.board.map(|v| v as f32))
    }
}

// 3. 简化MCTS
fn mcts(net: &MuZeroNetwork, state: &TicTacToe, sims: usize) -> usize {
    let root_state = state.to_tensor();
    let (hidden, (policy, _)) = net.initial_inference(&root_state);
    let legal_actions = state.legal_actions();

    // 选择最高先验概率的合法动作
    let probs = policy.softmax(0, Kind::Float);
    legal_actions
        .into_iter()
        .max_by_key(|&a| (probs.double_value(&[a as i64]) * 1e6) as i64)
        .unwrap()
}

// 4. 训练循环
fn main() {
    let mut net = MuZeroNetwork::new();
    let mut rng = rand::thread_rng();

    // 训练参数
    let lr = 1e-3;
    let batch_size = 32;
    let epochs = 100;

    let mut opt = nn::Adam::default().build(&net.vs, lr).unwrap();

    for epoch in 0..epochs {
        let mut total_loss = 0.0;

        for _ in 0..batch_size {
            let mut env = TicTacToe::new();
            let mut states = vec![];
            let mut actions = vec![];
            let mut rewards = vec![];

            // 自我对弈
            while !env.check_win() && env.legal_actions().len() > 0 {
                let action = mcts(&net, &env, 10);
                let (reward, done) = env.step(action);
                states.push(env.to_tensor());
                actions.push(action as i64);
                rewards.push(if done { reward } else { 0.0 });
            }

            // 计算目标价值
            let target_value = *rewards.last().unwrap_or(&0.0);

            // 反向传播
            for (state, action) in states.into_iter().zip(actions) {
                let (_, (policy_pred, value_pred)) = net.initial_inference(&state);
                let value_loss = (value_pred - target_value).pow_tensor_scalar(2).mean(Kind::Float);
                let policy_loss = -policy_pred
                    .log_softmax(0, Kind::Float)
                    .nll_loss(&Tensor::from(action));

                let loss = value_loss + policy_loss;
                opt.backward_step(&loss);
                total_loss += f64::from(loss.mean(Kind::Float));
            }
        }

        println!("Epoch {}: Loss={:.4}", epoch, total_loss / batch_size as f64);
    }

    // 测试AI
    let mut env = TicTacToe::new();
    while !env.check_win() && !env.legal_actions().is_empty() {
        let action = mcts(&net, &env, 50);
        env.step(action);
        println!("{:?}", env.board);
    }
}