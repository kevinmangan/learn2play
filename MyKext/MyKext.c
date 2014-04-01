//
//  MyKext.c
//  MyKext
//
//  Created by Kevin Mangan on 3/24/14.
//  Copyright (c) 2014 Kevin Mangan. All rights reserved.
//

#include <mach/mach_types.h>
#include <sys/systm.h>
#include <sys/kauth.h>
#include <sys/vnode.h>
#include <sys/proc.h>
#include <string.h>



// Lock
// static lck_mtx_t* gLock = NULL;

// Listener
static kauth_listener_t MyListener = NULL;

typedef struct {
    u_int32_t pid;
    char p_comm[MAXCOMLEN +1];
    char path[MAXPATHLEN];
}VNodeWatcherData_t;

static int listener(
                           kauth_cred_t    credential,
                           void *          idata,
                           kauth_action_t  action,
                           uintptr_t       arg0,
                           uintptr_t       arg1,
                           uintptr_t       arg2,
                           uintptr_t       arg3
                           ) {
    VNodeWatcherData_t data;
    int name_len = MAXPATHLEN;
    
    if (action==KAUTH_VNODE_EXECUTE) {
        //printf("MyKext: Listener found an execute action!\n");
        
        data.pid = vfs_context_pid((vfs_context_t)arg0);
        proc_name(data.pid, data.p_comm, MAXCOMLEN+1);
        
        if(vn_getpath((vnode_t)arg1, data.path, &name_len) == 0){
            
        }else{
            data.path[0] = '\0';
        }
       
        if(strncmp(data.p_comm, "Spotify", MAXCOMLEN+1) == 0){
            printf("MyKext: PID = %d, PROCNAME = %s\n", data.pid, data.p_comm);
            return KAUTH_RESULT_DENY;
        }
        
    }
    
    return KAUTH_RESULT_DEFER;
}


// Uninstall Listener
static kern_return_t uninstall_listener(void) {
    //lck_mtx_lock(gLock);
    
    if (MyListener) {
        kauth_unlisten_scope(MyListener);
        MyListener = NULL;
    }
    
    //lck_mtx_unlock(gLock);
    printf("MyKext: Listener Uninstalled.\n");
    return KERN_SUCCESS;
}


// Install Listener
static kern_return_t install_listener() {
    //lck_mtx_lock(gLock);
    
    MyListener = kauth_listen_scope(KAUTH_SCOPE_VNODE, listener, NULL);
    if (!MyListener) {
        printf("MyKext: failed to create KAUTH listener.\n");
        //lck_mtx_unlock(gLock);
        return KERN_FAILURE;
    }
    
    //lck_mtx_unlock(gLock);
    printf("MyKext: Listener Installed.\n");
    return KERN_SUCCESS;
}


kern_return_t MyKext_start(kmod_info_t * ki, void *d);
kern_return_t MyKext_stop(kmod_info_t *ki, void *d);

kern_return_t MyKext_start(kmod_info_t * ki, void *d)
{
    kern_return_t err;
    printf("MyKext HAS STARTED.\n");
    err = install_listener();
    return err;
}

kern_return_t MyKext_stop(kmod_info_t *ki, void *d)
{
    uninstall_listener();
    printf("MyKext HAS STOPPED.\n");
    return KERN_SUCCESS;
}
