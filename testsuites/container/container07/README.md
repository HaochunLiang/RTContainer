# container07：RTEMS 统一容器性能测试

本用例测量本项目 RTEMS 统一容器的启动、定时器唤醒和跨容器调度交接性能。
沿用前面 Docker/iSulad 的流程：预热 → 三轮正式采样 → 保存原始时间戳 →
每轮及合并统计 → 导出 CSV。代码已编写并做静态检查，**尚未编译或运行**。

## 测量口径

| 输出指标 | 起点与终点 | 单位 |
|---|---|---|
| `run_ready` | 配置、创建统一容器和任务之前 → 任务进入容器后的负载入口 | ns |
| `start_ready` | 容器和任务已创建，调用 `rtems_task_start()` 前 → 任务进入容器后的负载入口 | ns |
| `run_api_return` | 与 `run_ready` 相同起点 → `rtems_task_start()` 返回 | ns |
| `start_api_return` | 与 `start_ready` 相同起点 → `rtems_task_start()` 返回 | ns |
| `timer_wakeup` | `clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME)` 的计划截止时间 → 任务实际恢复执行 | ns |
| `switch_a_to_b` | A 发送事件前 → B 阻塞接收事件返回后 | ns |
| `switch_b_to_a` | B 回复事件前 → A 阻塞接收事件返回后 | ns |
| `switch_rtt` | A 发起 → A 收到 B 回复 | ns |
| `switch_half_rtt` | 完整往返除以 2，保留半纳秒精度 | ns |

“中断响应”沿用已选定的**定时器唤醒延迟**，包含时钟中断、定时器处理和任务调度，
不是纯硬件 IRQ 入口延迟。测试确实调用内核绝对定时睡眠，不用普通任务释放信号量模拟中断。

“容器切换”由两个独立统一容器各自的一条任务，在同一个 CPU 上通过 Classic Event
阻塞交接实现。两任务同为优先级 20，抢占开启、时间片关闭；唤醒同优先级对方后，
当前任务必须等待回复，才让对方运行。每次请求/回复都有序号检查。不是调用
pause/resume 的耗时，也不是单任务在两个命名空间之间反复 enter/leave 的耗时。

此仓库没有 Docker 的镜像、daemon、CLI 或 `container_start()` 接口。因此启动测量采用
`create container + create task + start task + enter container` 的实际链路；负载入口
时间戳在 `rtems_unified_container_enter()` 返回后立即采集。就绪通知、控制任务接收通知、
有效性断言和清理都不改变已采集的终点。`*_api_return` 是异步任务启动 API 返回时间，
不能当成 Linux CLI 返回时间。

RTEMS Event 与 Linux futex 不是同一种同步原语，RTEMS 任务/命名空间也不是 Linux 进程。
可以对照测试流程和结果量级，不能据此隔离出两种内核容器机制的纯开销。**QEMU 中的结果
只能用于该模拟环境，不能直接与此前 x86 主机上的 Docker/iSulad 数值比较。**

## 默认参数与资源

| 参数 | 每轮 | 三轮正式样本总数 |
|---|---:|---:|
| 创建并启动 | 预热 3 次，采样 30 次 | 90 |
| 已创建容器启动 | 预热 3 次，采样 30 次 | 90 |
| 定时器，周期 1 ms | 预热 1,000 次，采样 30,000 次 | 90,000 |
| 双容器事件往返 | 预热 1,000 次，采样 20,000 次 | 60,000 |

- `RTEMS_UNIFIED_CONTAINER_ALL`：PID、IPC、MNT、NET、UTS、CPU、MEM、IO 均启用。
  两个切换任务的 cgroup 和 PID 容器确实不同；任务进入/退出时检查全部命名空间绑定。
- 使用优先级调度器，CPU 0，最大处理器数 1。Classic 优先级 20 与 Linux FIFO 50 不存在
  数值上的等价关系，双方采用的是各自系统内的相同固定优先级、不分时间片的交接方式。
- CPU quota 和 period 都设为 1,000,000。本仓库实现将其直接送入 ticks watchdog，
  按 1 ms tick 约为 1,000 秒；用于让默认短测量阶段远离配额耗尽，并非关闭 cgroup。
- 内存配额 16 MiB，I/O 额度 1 GiB/s；计时循环不动态分配内存、不输出日志。
- 静态原始样本及排序缓冲区约 3.9 MiB，另配置 16 MiB executive RAM。
  统计在全部计时完成后执行，使用最近秩 P50/P95/P99、均值、最小值、最大值、样本标准差。
  不丢弃异常值；每轮和合并结果均输出。
- 64 位 CLOCK_MONOTONIC 纳秒时间戳避免累计使用较窄 CPU counter 导致长测量回绕。
  仍受 BSP 时钟与定时器精度影响；此实现的睡眠超时由时钟 tick 处理，默认 tick 1 ms。
  当前分支 `clock_getres(CLOCK_MONOTONIC)` 不支持时，记录 `unsupported`，不伪造分辨率。
- 每次事件接收有超时，控制任务也有阶段超时。定时器超期后仍推进原始绝对截止时间，
  保留追赶样本；另输出延迟达到一个周期的样本数和最大落后周期数。
- 每次启动测量使用全新容器/任务，不包含 RTEMS 系统启动和全局 BSD 网络栈初始化。
  每阶段结束先 leave，再删除任务和容器，避免绑定指针在任务退出后失效。

## 编译

在工程根目录操作。推荐使用独立配置和输出目录，不需要修改根目录 `config.ini`。
本目录配置只启用 container07，关闭容器日志以及 CPU/MEM/NET 监控。日志/监控宏若在其他
配置中启用，程序会在元数据里标记，结果会包含相应开销。

如果主机已有 `/opt/rtems6/bin/aarch64-rtems6-gcc`：

```bash
cd /home/neu/RTContainer
export PATH=/opt/rtems6/bin:$PATH

./waf configure \
  --rtems-config=testsuites/container/container07/config.ini \
  --rtems-tools=/opt/rtems6 \
  --prefix=/opt/rtems6 \
  --out=build-container07

./waf build --targets=testsuites/container/container07.exe -j$(nproc)
```

工具链安装在其他位置时，修改 PATH 和 `--rtems-tools`。`--prefix` 是安装前缀；本用例
直接运行生成的 ELF，**不需要 `./waf install`**。Waf configure 会切换本工程当前的构建配置；
以后恢复原工程构建时，再用原配置执行 configure。

如果沿用项目 README 的 Docker 交叉编译环境，可先在主机执行：

```bash
docker run --rm -it \
  --user "$(id -u):$(id -g)" \
  -v /home/neu/RTContainer:/work \
  -w /work \
  roker405/rtems6-env:v1.0 bash
```

进入后，工作目录已经是 `/work`，执行上面从 `export PATH=...` 开始的 configure/build 命令。
源代码和产物通过挂载保留在主机。该镜像的 PATH 已包含 `/opt/rtems6/bin`。

预期 ELF：

```text
/home/neu/RTContainer/build-container07/aarch64/a53_lp64_qemu/testsuites/container/container07.exe
```

新测试的构建注册位于 `spec/build/testsuites/container/container07.yml`，并已添加到
`spec/build/testsuites/container/grp.yml`。必须重新 configure 才能识别新增项；
`BUILD_CONTAINERTESTS`、容器组的全部功能开关及 `RTEMS_POSIX_API` 均需开启。

## QEMU 运行与结果导出

编译完成后，在安装有 `qemu-system-aarch64` 的主机终端运行：

```bash
cd /home/neu/RTContainer
set -o pipefail
qemu-system-aarch64 \
  -M virt,gic-version=3 \
  -cpu cortex-a53 -smp 1 -m 512M \
  -nographic -no-reboot \
  -kernel build-container07/aarch64/a53_lp64_qemu/testsuites/container/container07.exe \
  2>&1 | tee container07.log
```

正常过程包含 `*** BEGIN OF TEST CONTAINER 07 ***`、每轮进度、统计、原始数据、
`C07DONE` 和 `*** END OF TEST CONTAINER 07 ***`。不要在 `C07DONE` 前中断日志采集。
若 BSP 结束后 QEMU 未自行退出，看到 END 后按 Ctrl+A，再按 X 退出。

默认定时器采样自身约需 93 秒的目标时间，另有初始化、启动、切换和输出时间。
QEMU 的宿主负载和速度会影响墙钟耗时。**默认启用原始数据输出，约 150,180 行，串口输出
可能远慢于采样。** 所有原始数据均在三轮测量结束之后输出，不计入测量区间；
只看统计时可将 `C07_PRINT_RAW` 设为 0。

在主机上解析一次完整运行的日志：

```bash
python3 testsuites/container/container07/extract.py container07.log \
  --output container07-results
```

结果目录必须尚不存在。脚本会拒绝缺少 `C07DONE`、样本缺失、序号错误、负延迟、
时间戳乱序、周期漂移或统计与原始数据不一致的日志。默认导出：

- `summary.csv`：所有指标，每轮与合并统计，单位 ns；`round=0` 表示合并。
- `run.csv`、`start.csv`：t0=起点，t1=负载入口，t2=启动 API 返回，t3=0。
- `timer.csv`：t0=计划截止时间，t1=实际醒来，t2=t3=0。
- `switch.csv`：t0=A 发送前，t1=B 收到后，t2=B 回复前，t3=A 收到后。
- `manifest.json`：参数、输出完整性和定时器超期统计。

`C07_PRINT_RAW=0` 时只导出统计和元数据，并标记 `raw_validated=false`。
`summary.csv` 的 ns 除以 1,000 得到 µs，除以 1,000,000 得到 ms。
默认每项启动只有 90 个样本，nearest-rank P99 恰好等于观测最大值，不能视作稳定的极端尾延迟。

真实飞腾板测试需要将独立配置的 BSP 节替换成对应硬件 BSP，再用该板现有下载/启动方式
加载 ELF，串口日志仍可由同一脚本解析。不要把 `a53_lp64_qemu` ELF 直接当成 D2000 板的程序。

## 调整参数

参数都在 `benchmark.h`，也可通过配置文件的 `TEST_CONTAINER07_CPPFLAGS` 覆盖。
每次修改配置文件后重新 configure/build。例如仅验证流程的小样本配置：

```ini
TEST_CONTAINER07_CPPFLAGS = -DC07_ROUNDS=1u -DC07_STARTUP_SAMPLES=3u -DC07_STARTUP_WARMUP=1u -DC07_TIMER_SAMPLES=1000u -DC07_TIMER_WARMUP=100u -DC07_SWITCH_SAMPLES=1000u -DC07_SWITCH_WARMUP=100u -DC07_PRINT_RAW=0
```

这只是参数示例，不代表已执行过验证。正式对比保留默认参数。改变 tick 时，定时器周期
必须是 tick 的整数倍；增大样本数会增加静态内存占用及所需阶段时间。不要使用
`-icount` 把指令计数模拟的时间当作实际硬件性能。
