#include <stdio.h>
#include <unistd.h>
#include <glib.h>

GKeyFile* config_file;
gsize num_sections, curr_section;
gchar **groups;

int get_number_of_sections()
{
    return num_sections;
}

int load_config_file(char *filename)
{
    GError* error;
	config_file = g_key_file_new();
	error = NULL;

	if (!g_key_file_load_from_file(config_file, filename, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error)) {
		printf("ERROR %s\n", error->message);
        return 0;
	} else {
        groups = g_key_file_get_groups(config_file, &num_sections);
        curr_section=0;
        return 1;
    }
}

int get_next_section(char *section_name)
{
    while (curr_section < num_sections) {
        // Skip DEFAULT
        if (strncmp(groups[curr_section], "DEFAULT", 7) != 0) {
            strncpy(section_name, groups[curr_section], strlen(groups[curr_section]));
            curr_section++;
            return 1;
        } else {
            curr_section++;
        }
    }
    return 0;
}

int print_all_configs_no_section()
{
    GError* error = NULL;
    gsize num_keys;
    gchar **keys;
    guint key;
    gchar* value;

    keys = g_key_file_get_keys(config_file, NULL, &num_keys, &error);
    for(key = 0;key < num_keys;key++) {
        value = g_key_file_get_value(config_file, NULL, keys[key], &error);
        if (error == NULL) {
            printf("\t\tkey %u/%u: \t%s => %s", key, num_keys - 1, keys[key], value);
        } else{
            g_error_free (error);
            return 0;
        }
    }
    return 1;
}

int main(void)
{

    if (load_config_file("glibconfig.cfg") == OK) {
        char sec_name[255] = {0};
        int n_sections = get_number_of_sections();
        if (n_sections) {
            printf("Found %d sections \n", n_sections);
            while(get_next_section(sec_name) == OK) {
                printf("Found section %s \n", sec_name);
                memset(sec_name, 0, 255);
            }
            printf("All sections found, done\n");
        } else {
            print_all_configs_no_section();
        }
    }
	return 0;
}