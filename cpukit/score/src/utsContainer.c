#include <rtems/score/utsContainer.h>
#include <rtems/score/thread.h>
#include <rtems/rtems/tasks.h> 
#include <string.h>
#include <stdlib.h>
#include <rtems/score/threadimpl.h>

static int g_utsContainerId = 0;

// 后面在线程创建时，直接将它的container指针指向RootUtsContainer，之后在进行其他操作
int rtems_uts_container_initialize_root(UtsContainer **utsContainer)
{
    if (!utsContainer) return -1;

    *utsContainer = (UtsContainer *)malloc(sizeof(UtsContainer));
    if (!*utsContainer) return -1;

    (*utsContainer)->rc = 0;
    (*utsContainer)->ID = ++g_utsContainerId;
    strcpy((*utsContainer)->name, "root"); // 默认主机名

    return 0;
}

UtsContainer *rtems_uts_container_create(const char *name)
{
    UtsContainer *container = (UtsContainer *)malloc(sizeof(UtsContainer));
    if (!container) return NULL;
    container->rc = 0;
    container->ID = ++g_utsContainerId;
    if (name && name[0]) {
        strncpy(container->name, name, sizeof(container->name) - 1);
        container->name[sizeof(container->name) - 1] = '\0';
    } else {
        strcpy(container->name, "uts");
    }      

    rtems_uts_container_add_to_list(container);

    return container;
}

// 后续切换容器的时候要使用这个函数，避免容器rc为零之后无法自动删除
void rtems_uts_container_move_task(UtsContainer *srcContainer, UtsContainer *destContainer, Thread_Control *thread)
{
    if (!srcContainer || !destContainer || !thread) return;
    if (thread->container && thread->container->utsContainer == srcContainer) {
        thread->container->utsContainer = destContainer;
        srcContainer->rc--;
        if (srcContainer->rc <= 0) {
            rtems_uts_container_delete(srcContainer);
        }
        destContainer->rc++;
    }
}

// 回调函数，供删除UTS容器使用
static bool switch_to_root(Thread_Control *thread, void *arg)
{
    UtsContainer *utsContainer = (UtsContainer *)arg;
    Container *container = rtems_container_get_root();
    UtsContainer *root = container->utsContainer;

    if (thread->container && thread->container->utsContainer == utsContainer) {
        thread->container->utsContainer = root;
        root->rc++;
        utsContainer->rc--;
    }
    return true;
}

void rtems_uts_container_delete(UtsContainer *utsContainer)
{
    if (!utsContainer) return;
    Container *container = rtems_container_get_root();
    if (!container || !container->utsContainer) return;
    UtsContainer *root = container->utsContainer;
    if (utsContainer == root) return; // 根容器不能删

    // 遍历所有线程，将指向该容器的线程迁移到根容器
    rtems_task_iterate(switch_to_root, utsContainer);

    // 主动处理当前正在运行的线程，rtems_task_iterate没遍历到主线程（即正在执行的线程）
    Thread_Control *self = (Thread_Control *)_Thread_Get_executing();
    if (self && self->container && self->container->utsContainer == utsContainer) {
        self->container->utsContainer = root;
        root->rc++;
        utsContainer->rc--;
    }

    rtems_uts_container_remove_from_list(utsContainer);
    free(utsContainer);
}

UtsContainer *rtems_uts_container_find_by_thread(Thread_Control *thread)
{
    if (!thread) return NULL;
    return thread->container->utsContainer;
}

void rtems_uts_container_add_to_list(UtsContainer *utsContainer) {
    if (!utsContainer) return;
    Container *container = rtems_container_get_root();
    if (!container) return;

    UtsContainerNode **head = (UtsContainerNode **)&container->utsContainerListHead;
    UtsContainerNode *node = (UtsContainerNode *)malloc(sizeof(UtsContainerNode));
    if (!node) return;
    node->utsContainer = utsContainer;
    node->next = *head;
    *head = node;
}

void rtems_uts_container_remove_from_list(UtsContainer *utsContainer) {
    if (!utsContainer) return;
    Container *container = rtems_container_get_root();
    if (!container) return;

    UtsContainerNode **head = (UtsContainerNode **)&container->utsContainerListHead;
    UtsContainerNode **cur = head;
    while (*cur) {
        if ((*cur)->utsContainer == utsContainer) {
            UtsContainerNode *toRemove = *cur;
            *cur = toRemove->next;
            free(toRemove);
            break;
        }
        cur = &(*cur)->next;
    }
}

int rtems_uts_container_exists(UtsContainer *utsContainer)
{
    if (!utsContainer) return 0;
    Container *container = rtems_container_get_root();
    if (!container) return 0;
    if (container->utsContainer == utsContainer) return 1;
    UtsContainerNode *node = (UtsContainerNode *)container->utsContainerListHead;
    while (node) {
        if (node->utsContainer == utsContainer) return 1;
        node = node->next;
    }
    return 0;
}