#!/bin/sh

# list of header files, excluding private headers
 zero_header_list=`cd zero; ls gkyl_*.h | grep -v "priv" | sort; cd ..`
 app_header_list=`cd apps; ls gkyl_*.h | grep -v "priv" | sort; cd ..`

# Debug: print lists of headers
# echo "Zero headers: $zero_header_list"
# echo "App headers: $app_header_list"

cat <<EOF
#pragma once

/* Amalgamated include. Generated automatically during Gkeyll "make install" */

#ifdef __cplusplus
extern "C" {
#endif

EOF

for head in $zero_header_list
do
echo "#include <${head}>"
done

for head in $app_header_list
do
echo "#include <${head}>"
done

cat <<EOF1

#ifdef __cplusplus
}
#endif
EOF1

