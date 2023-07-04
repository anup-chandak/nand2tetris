/* Copyright 2017 Gerry Agbobada
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "vmTTools.h"

IOFiles open_filestreams(const char *filename) {
    IOFiles ioFiles;
    IOF_init(&ioFiles);

    // Input file stream
    ioFiles.fileCount = IOF_open_inputstream(&ioFiles, filename);

    // Output file stream and sets basename
    IOF_open_outputstream(&ioFiles, filename);
    IOF_set_basename(&ioFiles, filename);

    return ioFiles;
}

int IOF_open_inputstream(IOFiles *p_ioFiles, const char *filename) {
    int opened_files = 0;
    struct stat statbuf;
    stat(filename, &statbuf);

    if (S_ISDIR(statbuf.st_mode)) {
        DIR *dp;
        struct dirent *ep;
        dp = opendir(filename);

        if (dp != NULL) {
            while ((ep = readdir(dp))) {
                if (strstr(ep->d_name, ".vm") != NULL &&
                    strncmp(ep->d_name, ".", 1) != 0) {
                    char totalFilename[256];
                    strcpy(totalFilename, filename);
                    strcat(totalFilename, "/");
                    strcat(totalFilename, ep->d_name);
                    p_ioFiles->input[opened_files] = fopen(totalFilename, "r");
                    p_ioFiles->input_filenames[opened_files++] =
                        strdup(ep->d_name);
                }
            }

            (void)closedir(dp);
        } else
            perror("Couldn't open the directory");
    } else {
        p_ioFiles->input_filenames[opened_files] = strdup(filename);
        p_ioFiles->input[opened_files++] = fopen(filename, "r");
    }

    return opened_files;
}

FILE *IOF_open_outputstream(IOFiles *p_ioFiles, const char *filename) {
    char *filename_copy = NULL;
    char *filename_copy2 = NULL;
    char output_filename[50];
    output_filename[0] = '\0';
    filename_copy = strdup(filename);
    filename_copy2 = strdup(filename);
    char *file_basename = basename(filename_copy);
    strtok(file_basename, ".");
    // strtok modified file_basename
    p_ioFiles->staticName = strdup(file_basename);
    char *file_dirname = dirname(filename_copy2);
    strcat(output_filename, file_dirname);
    strcat(output_filename, "/");
    strcat(output_filename, file_basename);

    // Add another /DIRNAME if it's a dir.
    struct stat statbuf;
    stat(filename, &statbuf);
    if (S_ISDIR(statbuf.st_mode)) {
        strcat(output_filename, "/");
        strcat(output_filename, file_basename);
    }

    strcat(output_filename, ".asm");

    p_ioFiles->output = fopen(output_filename, "w");

    free(filename_copy);
    free(filename_copy2);

    return p_ioFiles->output;
}

void IOF_set_basename(IOFiles *p_ioFiles, const char *new_basename) {
    free(p_ioFiles->basename);

    char *filename_copy = NULL;
    filename_copy = strdup(new_basename);
    char *file_basename = basename(filename_copy);
    p_ioFiles->basename = strdup(strtok(file_basename, "."));

    free(filename_copy);
}

void IOF_clear(IOFiles *p_ioFiles) {
    free(p_ioFiles->basename);
    fclose(p_ioFiles->output);
    for (int i = 0; i < p_ioFiles->fileCount; ++i) {
        free(p_ioFiles->input_filenames[i]);
        fclose(p_ioFiles->input[i]);
    }
    free(p_ioFiles->input);
    free(p_ioFiles->input_filenames);
    free(p_ioFiles->staticName);
}

void IOF_init(IOFiles *p_ioFiles) {
    p_ioFiles->basename = NULL;
    p_ioFiles->output = NULL;
    p_ioFiles->input = calloc(MAX_NUMBER_OF_FILES, sizeof(FILE *));
    p_ioFiles->input_filenames = calloc(MAX_NUMBER_OF_FILES, sizeof(char *));
    p_ioFiles->fileCount = 0;
    p_ioFiles->staticName = NULL;
}

bool IOF_check(IOFiles *p_ioFiles) {
    if (p_ioFiles->basename == NULL) {
        return false;
    } else if (p_ioFiles->output == NULL) {
        return false;
    } else if (p_ioFiles->input == NULL) {
        return false;
    }
    return true;
}

void VMC_init(VMCommand *p_vmc) {
    p_vmc->command = calloc(MAX_COMMAND_WORDS, sizeof(char *));
    for (int i = 0; i < MAX_COMMAND_WORDS; ++i) {
        p_vmc->command[i] = calloc(MAX_COMMAND_WORD_LENGTH, sizeof(char *));
    }

    p_vmc->functionName = NULL;
}

void VMC_clear(VMCommand *p_vmc) {
    for (int i = 0; i < MAX_COMMAND_WORDS; ++i) {
        free(p_vmc->command[i]);
    }
    free(p_vmc->command);
    free(p_vmc->functionName);
}

void VMC_set_function_name(VMCommand *p_vmc, char *newFunctionName) {
    free(p_vmc->functionName);
    p_vmc->functionName = strdup(newFunctionName);
}
