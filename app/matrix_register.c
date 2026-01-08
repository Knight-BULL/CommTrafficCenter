#include "ctc_def.h"
#include "com_def.h"

typedef struct T_proc_socket_mag
{
    uint8_t pid;
    char socket_name[20];
}Proc_socket_map;

Proc_socket_map comm_matrix[] = 
{
    {.pid = REF_SEL_PROC, .socket_name = "/tmp/.ref_sel"},
};

char *get_comm_info_from_matrix(const uint8_t proc_id)
{
    int num = sizeof(comm_matrix)/sizeof(Proc_socket_map);

    if (num > 0)
    {
        for (int i = 0; i < num; i++)
        {
            if (comm_matrix[i].pid == proc_id)
            {
                return comm_matrix[i].socket_name;
            }
        }
    }

    return NULL;
}
