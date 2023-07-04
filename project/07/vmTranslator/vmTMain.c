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
#include "vmTMain.h"

int main(int argc, char **argv) {
    IOFiles ioFiles;
    if (argc > 2) {
        printf("Too many arguments supplied.\n");
        return 1;
    } else if (argc <= 1) {
        printf("One argument expected.\n");
        return 1;
    } else {
        ioFiles = open_filestreams(argv[1]);
        if (!IOF_check(&ioFiles)) {
            fprintf(stderr, "Problem in IOFiles");
            return 1;
        }
    }

    // Allocate filestream and parse it
    LabelCounter LabelCounter;
    LC_init(&LabelCounter);
    VMCommand cmd;
    VMC_init(&cmd);

    // Allocation of resources for the translation
    char line[LINE_BUFFERSIZE];

    // Add init only if there are multiple files
    if (ioFiles.fileCount > 1) {
        fprintf(ioFiles.output, "%s", init_asm);
    }

    // .vm file parsing loop
    for (int i = 0; i < ioFiles.fileCount; i++) {
        IOF_set_basename(&ioFiles, ioFiles.input_filenames[i]);
        VMC_set_function_name(&cmd, ioFiles.basename);
        while (fgets(line, LINE_BUFFERSIZE - 1, ioFiles.input[i]) != NULL) {
            int command_length = parse_line(line, &cmd);
            // Skip the line if it is a comment
            if (command_length == 0) {
                continue;
            } else {
                // This is where we should call the writing functions
                const char *asm_dict_file =
                    choose_asm_dict_file(&cmd, command_length);
                if (asm_dict_file == NULL) {
                    fprintf(stderr, "No stub found ! File : %s Command : %s\n",
                            ioFiles.input_filenames[i], line);

                    for (int j = 0; j < command_length; ++j) {
                        fprintf(stderr, "%s ", cmd.command[j]);
                    }
                    fprintf(stderr, "\n");
                    exit(1);
                }
                write_to_file(ioFiles.output, &cmd, &LabelCounter,
                              asm_dict_file, ioFiles.basename,
                              ioFiles.basename);
            }
        }
    }

    // Cleanup
    IOF_clear(&ioFiles);
    VMC_clear(&cmd);

    return 0;
}
