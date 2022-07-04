export LD_PRELOAD=/usr/lib64/libstdc++.so.6.0.19
matlab -nodisplay -nojvm -nosplash -nodesktop -r \
        "try, parametric_driver, catch e, disp(getReport(e)), exit(1), end, exit(0);"
      echo "matlab exit code: $?"
