#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>

int main() {
    struct statfs *mounts;
    int count = getmntinfo(&mounts, MNT_WAIT);

    if (count == 0) {
        perror("getmntinfo");
        return 1;
    }

    printf("Available logical devices:\n");

    for (int i = 0; i < count; i++) {
        printf("%s on %s (%s)\n", mounts[i].f_mntfromname, 
mounts[i].f_mntonname, mounts[i].f_fstypename);
    }

    return 0;
}

