#include <usbhsfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

void getLabel(s32 device_id, u8 lun)
{
    char label[50];
    if(usbHsFsGetLabel(device_id, lun, label)) printf("Drive label: '%s'\n", label);
    else printf("Error getting drive label...\n");
    consoleUpdate(NULL);
}

void setLabel(s32 device_id, u8 lun)
{
    const char *new_label = "DEMO-LABEL";
    printf("Setting drive label to '%s'...\n", new_label);
    if(usbHsFsSetLabel(device_id, lun, new_label)) printf("New label correctly set!\n");
    else printf("Error setting new label...\n");
    consoleUpdate(NULL);
}

void fsTest(s32 device_id, u8 lun)
{
    /* We are only mounting one drive at a time, so it's guaranteed to be usb-0. */

    /* List all files in root. */
    DIR *dp = opendir("usb-0:/");
    if(dp)
    {
        printf("Listing root files/dirs:\n");
        consoleUpdate(NULL);
        while(true)
        {
            struct dirent *dt = readdir(dp);
            if(!dt) break;

            printf(" - ");
            if(dt->d_type & DT_DIR) printf("[D] ");
            else printf("[F] ");
            printf("usb-0:/%s\n", dt->d_name);
            consoleUpdate(NULL);
        }
        closedir(dp);
    }
    else
    {
        printf("Unable to open root dir... errno value: %d\n", errno);
        consoleUpdate(NULL);
    }

    /* Log to a file. */
    FILE *fp = fopen("usb-0:/sample.txt", "w");
    if(fp)
    {
        printf("Opened sample txt file - logging...\n");
        fprintf(fp, "Hello %s!", "world");
        fclose(fp);
        printf("Logged sample message!\n");
        consoleUpdate(NULL);
    }
    else
    {
        printf("Error opening file... errno value: %d\n", errno);
        consoleUpdate(NULL);
    }
    
    /* Read logged file. */
    fp = fopen("usb-0:/sample.txt", "r");
    if(fp)
    {
        printf("Reading logged text...\n");
        char log_text[50];
        fscanf(fp, "%s", log_text);
        printf("Logged text: '%s'...\n", log_text);
        fclose(fp);
        consoleUpdate(NULL);
    }
    else
    {
        printf("Error opening file... errno value: %d\n", errno);
        consoleUpdate(NULL);
    }

    /* Stat file. */
    struct stat st;
    printf("Statting sample file...\n");
    if(stat("usb-0:/sample.txt", &st) == 0)
    {
        if(st.st_mode & S_IFREG) printf("It's a file!\n");
        else if(st.st_mode & S_IFDIR) printf("It's a dir...?\n");
        consoleUpdate(NULL);
    }
    else
    {
        printf("Error statting file... errno value: %d\n", errno);
        consoleUpdate(NULL);
    }
}

bool waitConfirmation()
{
    printf("Press A to confirm, any other key to exit\n\n");
    consoleUpdate(NULL);

    while(appletMainLoop())
    {
        hidScanInput();

        u64 k = hidKeysDown(CONTROLLER_P1_AUTO);
        if(k & KEY_A) return true;
        else if(k) return false;
    }
    return false;
}

void listTestDrives() {
    u32 drive_count = usbHsFsGetDriveCount();
    printf("Found %d devices...\n", drive_count);
    consoleUpdate(NULL);

    s32 *drive_array = (s32*)malloc(sizeof(s32) * drive_count);
    memset(drive_array, 0, sizeof(s32) * drive_count);

    drive_count = usbHsFsListDrives(drive_array, drive_count);
    printf("Listed %d devices...\n", drive_count);
    consoleUpdate(NULL);

    for(u32 i = 0; i < drive_count; i++)
    {
        s32 drive_id = drive_array[i];
        printf("Drives[%d] -> ID %d\n", i, drive_id);
        consoleUpdate(NULL);

        printf("Would you like to test this drive?\n");
        if(waitConfirmation())
        {
            u8 max_lun = 0;
            if(usbHsFsGetDriveMaxLUN(drive_id, &max_lun))
            {
                for(u8 j = 0; j < max_lun; j++)
                {
                    printf("Would you like to test with LUN %d?\n", j);
                    if(waitConfirmation())
                    {
                        u32 mount_idx = 0;
                        if(usbHsFsMount(drive_id, j, &mount_idx))
                        {
                            printf("Mounted drive LUN as 'usb-%d:/'!\n", mount_idx);
                            consoleUpdate(NULL);

                            printf("Press A to get label\nPress X to set label\nPress Y for filesystem test\nPress any other key to skip\n");
                            consoleUpdate(NULL);
                            while(appletMainLoop())
                            {
                                hidScanInput();

                                u64 k = hidKeysDown(CONTROLLER_P1_AUTO);
                                if(k & KEY_A) getLabel(drive_id, j);
                                else if(k & KEY_X) setLabel(drive_id, j);
                                else if(k & KEY_Y) fsTest(drive_id, j);
                                else if(k) break;
                            }

                            printf("Unmounting drive LUN...\n");
                            usbHsFsUnmount(drive_id, j);
                        }
                        else {
                            printf("Unable to mount LUN...\n");
                            consoleUpdate(NULL);
                        }   
                    }

                }
            }
            else
            {
                printf("Unable to get device's max LUN...\n");
                consoleUpdate(NULL);
            }
        }
    }
}

int main()
{
    consoleInit(NULL);
    usbHsFsInitialize();

    printf("usbHsFs test - press A to list drives, press + to exit!\n");
    consoleUpdate(NULL);
    
    while(appletMainLoop())
    {
        hidScanInput();

        u64 k = hidKeysDown(CONTROLLER_P1_AUTO);
        if(k & KEY_A) listTestDrives();
        if(k & KEY_PLUS) break;
    }

    consoleExit(NULL);
    usbHsFsExit();
    return 0;
}