# Create an 32-bit x86 Architecture Operating System
> *Source Code* ini dibuat oleh kami, Kelompok ApaGaKeOS, untuk memenuhi Tugas Besar Sistem Operasi yaitu membuat
> Sistem operasi yang akan dibuat akan berjalan pada arsitektur x86 32-bit

## Daftar Isi
- [Author](#author)
- [Deskripsi Singkat](#deskripsi-singkat)
- [Sistematika File](#sistematika-file)
- [Requirements](#requirements)
- [Cara Mengkompilasi dan Menjalankan Program](#cara-mengkompilasi-dan-menjalankan-program)

## Author
| NIM      | Nama                       | Github Profile                                            |
| -------- | ---------------------------|-----------------------------------------------------------|
| 13521062 | Go Dillon Audris           | [GoDillonAudris512](https://github.com/GoDillonAudris512) |
| 13521084 | Austin Gabriel Pardosi     | [AustinPardosi](https://github.com/AustinPardosi)         |
| 13521108 | Michael Leon Putra Widhi   | [mikeleo03](https://github.com/mikeleo03)                 |
| 13521172 | Nathan Tenka               | [Nat10k](https://github.com/Nat10k)                       |

## Deskripsi Singkat
Tugas ini akan membuat sebuah program *mistis* yang umumnya tidak diketahui orang awam bernama sistem operasi. Sistem operasi yang akan dibuat akan berjalan pada arsitektur x86 32-bit yang nanti akan dijalankan dengan emulator QEMU. Tugas ini akan dibagi menjadi beberapa milestone.

### Milestone 1 - Booting, Kernel, 32 bit Protected Mode
Waktu implementasi : Jumat, 10 Februari 2023 - Kamis, 2 Maret 2023
1. Menyiapkan alat dan *repository*
2. Pembuatan *build script*
3. Menjalankan sistem operasi
4. Membuat *output* dengan *text*
5. Memasuki *Protected Mode*

### Milestone 2 - Interrupt, Driver, dan Filesystem
Waktu implementasi : Jumat, 3 Maret 2023 - Kamis, 30 Maret 2023
1. Interrupt dan IDT
2. Keyboard driver
3. Disk driver
4. *File System* FAT32
5. [BONUS] Dukungan CMOS time untuk *file system*

### Milestone 3 - Paging, User Mode, dan Shell
Waktu implementasi : Jumat, 31 Maret 2023 - Sabtu, 29 April 2023
1. Paging
2. User Mode
3. Shell
4. [BONUS] *Relative pathing*
5. [BONUS] Kreativitas, diantaranya
    1. Pemrosesan perintah ganda (lebih dari 2 argumen) untuk mkdir, cat, dan whereis
    2. Pemrosesan perintah ls lebih dari 1 argumen
    3. Penanganan terhadap masukan dengan maupun tanpa ekstensi untuk *file*

## Sistematika File
```bash
.
├─── bin
├─── other
├─── src
│   ├─── filesystem
│   ├─── framebuffer
│   ├─── gdt
│   ├─── inserter
│   ├─── interrupt
│   ├─── kernel
│   ├─── keyboard
│   ├─── paging
│   ├─── portio
│   ├─── std
│   ├─── user
│   ├─── linker.ld
│   └─── menu.lst
├─── makefile
└─── README.md
```

## Requirements
- GCC compiler (versi 11.2.0 atau yang lebih baru)
- Visual Studio Code
- Windows Subsystem for Linux (WSL2) dengan distribusi minimal Ubuntu 20.04
- Emulator QEMU

## Cara Mengkompilasi dan Menjalankan Program
1. Lakukan *clone repository* melalui terminal dengan *command* berikut
    ``` bash
    $ git clone https://github.com/Sister20/if2230-2023-apagakeos.git
    ```
2. Lakukan eksekusi pada makefile dengan memasukkan *command* `make all` pada terminal. Jika berhasil maka akan tercipta beberapa file pada folder `bin`
3. Jalan sistem oprerasi dengan membuka Visual Studio Code dan jalankan `Shift + F5`. Pastikan QEMU yang digunakan sudah aktif sebelumnya. Jika proses aktivasi tidak berhasil, maka gunakan [Panduan Debugger dan WSL](https://docs.google.com/document/d/1Zt3yzP_OEiFz8g2lHlpBNNr9qUyXghFNeQlAeQpAaII/edit#). 

Jika berhasil, maka sistem operasi akan muncul pada layar.