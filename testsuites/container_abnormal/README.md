# 容器异常测试

每个用例位于独立的 `container_abnormalXX/init.c`，沿用
`testsuites/container` 的 RTEMS Init 任务、断言和 YAML 构建注册方式。
源码、构建配置及运行脚本在工作区维护；编译和运行由用户在工具链容器中执行。

| 用例 | 对应需求 | 实际检查 |
| --- | --- | --- |
| container_abnormal01 | 5.6 创建异常与资源回滚 | 在 PID、UTS、MNT、NET、IPC、cgroup、IO cgroup 创建入口逐个注入失败，每种重复三轮；检查错误码、空输出指针、命名空间注册链表、cgroup ID、IO cgroup 数量、堆/工作区/RTEMS 对象资源快照、根容器主机名和消息队列；解除故障后重新创建、进入、退出、删除。 |
| container_abnormal02 | 5.3 非法命令与异常状态操作 | `/cpuctl` 非法命令、缺失/非法 ID、重复 pause、未 pause 就 resume、删除后的 ID 操作；严格要求拒绝并给出错误，检查容器状态和另一个容器的工作任务应答；验证正常 pause/resume 后任务恢复。 |
| container_abnormal03 | 5.1.5 NET 接口故障隔离与恢复 | 两个 NET 容器分别绑定同一 UDP 地址/端口；仅替换第一个容器 lo0 的输出回调，使驱动返回 ENETDOWN；验证真实 sendto/recvfrom 失败、第二个容器收发正常、恢复原回调后原 socket 继续通信。 |
| container_abnormal04 | 5.2.3 IO Cgroup 块设备异常 | 两个工作任务分别进入独立 IO cgroup，访问各自 RAM Disk；仅让第一个磁盘的真实驱动读请求完成为 RTEMS_IO_ERROR；每次读前清除缓存，验证错误传播、第二个任务读写和计数不受干扰、故障解除后原数据可读且新数据可写，重复三轮。 |
| container_abnormal05 | 5.4 日志缓冲区溢出与输出异常 | 向 8 条容量的环形缓冲区连续写入 257 条日志，逐条核对最后 8 条及顺序、覆盖数量 249；再让文件输出目标的 IMFS write 返回 EIO，验证内存日志继续记录、恢复后原文件输出目标继续写入。 |

故障注入均在测试程序内：01 使用链接器 `--wrap`，03 使用指定接口的驱动回调，
04 使用 RAM Disk 的 ioctl 回调，05 使用 IMFS 设备节点。01 还要求 cgroup 删除
路径在释放控制块前取消 CPU watchdog；该修复位于 `cpukit`，否则回滚后 watchdog
会继续访问已释放的 cgroup。
网络测试使用独立的虚拟回环接口，无需宿主机 TAP、网桥或外网。
磁盘和日志目标都在 RTEMS 内存中，无需额外磁盘镜像。

## 编译

在装有 RTEMS 6 AArch64 交叉工具链的容器中执行。工具链路径沿用仓库 README
的 `/opt/rtems6`；如果实际安装位置不同，修改下面的 `--prefix`，或另外指定
`--rtems-tools=/实际工具链目录`。

```sh
cd /home/neu/RTContainer
./waf configure \
  --rtems-config=testsuites/container_abnormal/config.ini \
  --prefix=/opt/rtems6 \
  --out=build-container-abnormal
./waf build -j"$(nproc)"
```

配置启用独立开关 `BUILD_CONTAINERABNORMALTESTS`，并启用全部所需容器模块、
`RTEMSCFG_CONTAINER_FILE` 和 `RTEMSCFG_CONTAINER_LOG`。
仅选择本组五个用例，BSP 为 `aarch64/a53_lp64_qemu`，采用单核配置。
本仓库的这个 BSP 不开放 `RTEMS_SMP` 配置选项，无需添加 `RTEMS_SMP = False`。
如果旧配置显示 `Unknown configuration option: RTEMS_SMP`，删除该行后重新
configure 即可；该提示不是配置失败，也不是运行时崩溃的原因。
输出文件为：

```text
build-container-abnormal/aarch64/a53_lp64_qemu/testsuites/container_abnormal/container_abnormal01.exe
build-container-abnormal/aarch64/a53_lp64_qemu/testsuites/container_abnormal/container_abnormal02.exe
build-container-abnormal/aarch64/a53_lp64_qemu/testsuites/container_abnormal/container_abnormal03.exe
build-container-abnormal/aarch64/a53_lp64_qemu/testsuites/container_abnormal/container_abnormal04.exe
build-container-abnormal/aarch64/a53_lp64_qemu/testsuites/container_abnormal/container_abnormal05.exe
```

配置完成后，也可以只编译某一个用例，例如：

```sh
./waf build -j"$(nproc)" \
  --targets=testsuites/container_abnormal/container_abnormal03.exe
```

不需要执行 `waf install`。如果在同一源码目录切回其他构建配置，需要再次执行
对应配置的 `waf configure`。

## 运行

需要 `qemu-system-aarch64` 和 GNU coreutils 的 `timeout`。
一次运行全部五个用例：

```sh
cd /home/neu/RTContainer
sh testsuites/container_abnormal/run.sh
```

脚本逐一启动 QEMU，将完整输出保存到可执行文件目录下的 `logs/`，并逐项输出
PASS/FAIL。任何一个测试未通过，脚本最终返回非零。可用第一个参数指定其他
可执行文件目录，或通过 `QEMU` 环境变量指定 QEMU 程序路径。

每个用例有 30 秒 RTEMS 任务看门狗；脚本另设 45 秒宿主机超时，以处理关中断
死锁、崩溃或 BSP 在测试结束后停机但未退出 QEMU 的情况。后者可能等待到宿主机
超时才启动下一个用例，五个用例总计可能需要约四分钟。

手动运行单个用例（将 `01` 替换成 `02`、`03`、`04` 或 `05`）：

```sh
qemu-system-aarch64 -M virt,gic-version=3 \
  -cpu cortex-a53 -smp 1 -m 512M \
  -nographic -no-reboot \
  -kernel build-container-abnormal/aarch64/a53_lp64_qemu/testsuites/container_abnormal/container_abnormal01.exe
```

成功应出现 `[result] contract failures: 0` 和对应的
`*** END OF TEST CONTAINER ABNORMAL 01 ***`。如果 QEMU 未退出，按 `Ctrl-a` 后按
`x`。出现 `[FAIL]`、断言失败、看门狗超时或缺少 END 标记都不能判为通过；
不能仅凭 QEMU 的退出码判断 RTEMS 测试结果。

若发生 CPU 异常，脚本会使用 `/opt/rtems6/bin/aarch64-rtems6-addr2line`
和刚运行的同一份 `.exe` 自动解析 PC/LR；源码位置同时保存到
`logs/container_abnormalXX.addr2line.log`。工具路径可通过 `ADDR2LINE` 覆盖。
请保留原始日志和地址解析输出，重新编译后旧地址可能不再对应原源码位置。

01 的 NET 回归还检查恢复后的 UDP/TCP socket 创建和关闭，在故障回滚及
正常恢复删除后分别检查堆完整性、堆/工作区用量及文件数。预热会执行完整的
创建、进入、退出和删除，以排除线程上下文的一次性分配。

对应内核修复包括初始化 UDP/TCP PCB 哈希桶，以及回收尚无收发流量、无外部
引用的 loopback 回滚资源（组播成员、接口地址、接口、空路由表）。有业务流量、
veth 或外部引用的接口仍需要其各自的完整 detach 路径，本组 01 不覆盖该场景。

## 当前接口的预期差异

- 02、05 为使用双倍最小栈的任务显式配置了额外栈空间；03 允许未配置宿主
  网络接口时根 NET 容器的接口指针为空。启动阶段的任务创建失败或根接口
  断言失败不属于预期的故障注入结果。
- 02 是针对需求的严格回归测试。`containerfs.c` 和 cgroup 状态检查现已修复，
  非法操作应返回错误；通过时应输出 `contract failures: 0`。
- 01 现在按故障阶段逐级启用模块，避免把后续模块的清理问题混入当前阶段。
  资源检查比较堆、工作区和打开文件数；日志中的逐类对象计数作为诊断信息。
- 04 的 `rtems_io_cgroup_handle_request()` 只负责准入和统计；实际读失败来自
  RAM Disk，经 `rtems_bdbuf_read()` 返回。这里检查的是读请求故障，不将
  cgroup 准入成功或缓存命中视为设备 IO 成功。
- 05 的公开日志接口没有丢弃计数查询，测试以保留日志的连续序号和容量推导、
  核对覆盖数量；这不等于验证内核维护的独立丢弃计数器。当前日志插入接口也
  不返回文件写错误，测试通过故障设备收到的失败写请求确认故障确实发生。

若 01 出现 CPU 异常，需要使用产生该日志时的同一份 `.exe` 解析 PC/LR，不能
用重新编译后的文件解析旧地址。例如，用户首轮日志中的地址可这样解析：

```sh
/opt/rtems6/bin/aarch64-rtems6-addr2line -a -f -C \
  -e build-container-abnormal/aarch64/a53_lp64_qemu/testsuites/container_abnormal/container_abnormal01.exe \
  0x400317d8 0x40019770
```
