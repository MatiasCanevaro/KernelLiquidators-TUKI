# tp-2023-1c-Kernel-Liquidators

git clone https://github.com/sisoputnfrba/so-deploy.git

cd so-deploy

./deploy.sh -r=release -l=mumuki/cspec -p=shared -p=consola -p=kernel -p=memoria -p=filesystem -p=cpu tp-2023-1c-Kernel-Liquidators  
  
TOKEN PRIMERA ENTREGA  
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

enunciado: https://docs.google.com/document/d/1orfThJsPmMx5uPzbY3wClGhqX8jASMOCUMlWnYAr7cA/edit?pli=1
enunciado pruebas: https://docs.google.com/document/d/1MNalaTCB95qGO8q3rlR7VVCQqv3VLP3oeYxBgXgBy5g/edit

-----CONSOLA------

cambiar ips

git clone https://github.com/sisoputnfrba/tuki-pruebas.git


PRUEBA BASE  
./bin/consola.out ../../tuki-pruebas/BASE_1& ./bin/consola.out ../../tuki-pruebas/BASE_2& ./bin/consola.out ../../tuki-pruebas/BASE_2&

PRUEBA DEADLOCK  
./bin/consola.out ../../tuki-pruebas/DEADLOCK_1& ./bin/consola.out ../../tuki-pruebas/DEADLOCK_2& ./bin/consola.out ../../tuki-pruebas/DEADLOCK_3&

./bin/consola.out ../../tuki-pruebas/DEADLOCK_4

PRUEBA MEMORIA  
./bin/consola.out ../../tuki-pruebas/MEMORIA_1& ./bin/consola.out ../../tuki-pruebas/MEMORIA_2& ./bin/consola.out ../../tuki-pruebas/MEMORIA_3&

PRUEBA FILESYSTEM  
./bin/consola.out ../../tuki-pruebas/FS_1& ./bin/consola.out ../../tuki-pruebas/FS_2&

./bin/consola.out ../../tuki-pruebas/FS_3& ./bin/consola.out ../../tuki-pruebas/FS_3& ./bin/consola.out ../../tuki-pruebas/FS_3& ./bin/consola.out ../../tuki-pruebas/FS_3&

PRUEBA ERRORES  
./bin/consola.out ../../tuki-pruebas/ERROR_1& ./bin/consola.out ../../tuki-pruebas/ERROR_2& ./bin/consola.out ../../tuki-pruebas/ERROR_3& ./bin/consola.out ../../tuki-pruebas/ERROR_4&

------KERNEL------

cambiar ips

PRUEBA BASE  
./bin/kernel.out cfg/fifo.config

./bin/kernel.out cfg/hrrn.config

PRUEBA DEADLOCK  
./bin/kernel.out cfg/deadlock.config

PRUEBA MEMORIA  
./bin/kernel.out cfg/memoria.config

PRUEBA FILESYSTEM  
./bin/kernel.out cfg/fs.config

PRUEBA ERRORES  
./bin/kernel.out cfg/errores.config

-------CPU--------

cambiar ips

PRUEBA BASE  
./bin/cpu.out cfg/base.config

PRUEBA DEADLOCK  
./bin/cpu.out cfg/deadlock.config

PRUEBA MEMORIA  
./bin/cpu.out cfg/memoria.config

PRUEBA FILESYSTEM  
./bin/cpu.out cfg/fs.config

PRUEBA ERRORES  
./bin/cpu.out cfg/errores.config

-----MEMORIA------

cambiar ips

PRUEBA BASE  
./bin/memoria.out cfg/base.config

PRUEBA DEADLOCK  
./bin/memoria.out cfg/deadlock.config

PRUEBA MEMORIA  
./bin/memoria.out cfg/first.config

./bin/memoria.out cfg/best.config

./bin/memoria.out cfg/worst.config

PRUEBA FILESYSTEM  
./bin/memoria.out cfg/fs.config

PRUEBA ERRORES  
./bin/memoria.out cfg/errores.config

----FILESYSTEM----

cambiar ips

TODAS LAS PRUEBAS
./bin/filesystem.out cfg/filesystem.config
