/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2019-2021 Xilinx, Inc. All rights reserved.
 */

#ifndef _XRT_VERSION_H_
#define _XRT_VERSION_H_

static const char xrt_build_version[] = "2.13.0";

static const char xrt_build_version_branch[] = "2022.1";

static const char xrt_build_version_hash[] = "2a6dc026480914ea1c9f02977a6ab4b57e8a3c8d";

static const char xrt_build_version_hash_date[] = "Tue, 12 Apr 2022 20:01:55 +0530";

static const char xrt_build_version_date_rfc[] = "Tue, 19 Apr 2022 14:04:46 +0000";

static const char xrt_build_version_date[] = "2022-04-12 14:31:55";

static const char xrt_modified_files[] = "";

#define XRT_DRIVER_VERSION "2.13.0,2a6dc026480914ea1c9f02977a6ab4b57e8a3c8d"

#define XRT_VERSION(major, minor) ((major << 16) + (minor))
#define XRT_VERSION_CODE XRT_VERSION(2, 13)
#define XRT_MAJOR(code) ((code >> 16))
#define XRT_MINOR(code) (code - ((code >> 16) << 16))

# ifdef __cplusplus
#include <iostream>
#include <string>

namespace xrt {

class version {
 public:
  static void print(std::ostream & output)
  {
     output << "       XRT Build Version: " << xrt_build_version << std::endl;
     output << "    Build Version Branch: " << xrt_build_version_branch << std::endl;
     output << "      Build Version Hash: " << xrt_build_version_hash << std::endl;
     output << " Build Version Hash Date: " << xrt_build_version_hash_date << std::endl;
     output << "      Build Version Date: " << xrt_build_version_date_rfc << std::endl;

     std::string modifiedFiles(xrt_modified_files);
     if ( !modifiedFiles.empty() ) {
        const std::string& delimiters = ",";      // Our delimiter
        std::string::size_type lastPos = 0;
        int runningIndex = 1;
        while(lastPos < modifiedFiles.length() + 1) {
          if (runningIndex == 1) {
             output << "  Current Modified Files: ";
          } else {
             output << "                          ";
          }
          output << runningIndex++ << ") ";

          std::string::size_type pos = modifiedFiles.find_first_of(delimiters, lastPos);

          if (pos == std::string::npos) {
            pos = modifiedFiles.length();
          }

          output << modifiedFiles.substr(lastPos, pos-lastPos) << std::endl;

          lastPos = pos + 1;
        }
     }
  }
};
}
#endif

#endif
