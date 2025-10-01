use plotters::prelude::*;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // 创建svg图片对象
    let root = SVGBackend::new("plot.svg", (640, 480)).into_drawing_area();
    // 图片对象的背景颜色填充
    root.fill(&WHITE)?;
    // 创建绘图对象
    let mut chart = ChartBuilder::on(&root)
        // 图表名称  (字体样式, 字体大小)
        .caption("水平柱状图", ("sans-serif", 30))
        // 图表左侧与图片边缘的间距
        .set_label_area_size(LabelAreaPosition::Left, 40)
        // 图表底部与图片边缘的间距
        .set_label_area_size(LabelAreaPosition::Bottom, 40)
        // 构建二维图像, x轴 0.0 - 10.0； y轴 0.0 - 10.0；
        // 重点！！！！ 这里的传值必须为SegmentedCoord类型
        .build_cartesian_2d(0..50, (0..10).into_segmented())?;
    // 配置网格线
    chart.configure_mesh().draw()?;

    // 数值
    let data = [25, 37, 15, 32, 45, 33, 32, 10, 0, 21, 5];
    // y轴
    let index = vec![0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10];

    chart.draw_series(
        // 循环x轴数据
        // 压缩y轴生成结构
        data.iter().zip(index.iter()).map(
            |(x, y)| {
                // 创建y轴左部分段对象
                let y0 = SegmentValue::Exact(*y);
                // 创建y轴右部分段对象
                let y1 = SegmentValue::Exact(*y + 1);
                let mut bar = Rectangle::new([(0, y0), (*x, y1)], GREEN.filled());
                // 设置上下柱间距
                bar.set_margin(5, 5, 0, 0);
                bar
            }
        ))?;
    Ok(())
}