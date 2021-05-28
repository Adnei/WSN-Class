#!/bin/bash

rm -rf *.log
rm -rf *.testlog

for i in {1..10}
do
  java -jar ${HOME}/contiki/tools/cooja/dist/cooja.jar -nogui=../rpl_simulation_v2.csc -contiki=${HOME}/contiki
  sed -i '/test/Id' "COOJA.testlog"
  mv "COOJA.testlog" "exec_${i}.log"
  sed -i '3,$'"s/.*/$i|&/" "exec_${i}.log"
  if (($i > 1)); then
    sed -i '1,2d' "exec_${i}.log"
  else
    sed -i '1d' "exec_${i}.log"
  fi
  cat "exec_${i}.log" >> "all_logs.log"
done

sed -i 1's/.*/EXEC|&/' "all_logs.log"
