#include <fstream>
#include <iostream>

#include <QPainter>
#include "emuwindow.hpp"

using namespace std;

ifstream::pos_type filesize(const char* filename)
{
    ifstream in(filename, ifstream::ate | ifstream::binary);
    return in.tellg();
}

EmuWindow::EmuWindow(QWidget *parent) : QMainWindow(parent)
{

}

int EmuWindow::init()
{
    //Initialize emulator
    e.reset();
    ifstream BIOS_file("bios.bin", ios::binary | ios::in);
    if (!BIOS_file.is_open())
    {
        printf("Failed to load PS2 BIOS.\n");
        return 1;
    }
    printf("Loaded PS2 BIOS.\n");
    uint8_t* BIOS = new uint8_t[1024 * 1024 * 4];
    BIOS_file.read((char*)BIOS, 1024 * 1024 * 4);
    BIOS_file.close();
    e.load_BIOS(BIOS);
    delete[] BIOS;
    BIOS = nullptr;
    const char* file_name = "3stars.elf";

    ifstream ELF_file(file_name, ios::binary | ios::in);
    if (!ELF_file.is_open())
    {
        printf("Failed to load %s\n", file_name);
        return 1;
    }
    long long ELF_size = filesize(file_name);
    uint8_t* ELF = new uint8_t[ELF_size];
    ELF_file.read((char*)ELF, ELF_size);
    ELF_file.close();

    printf("Loaded %s\n", file_name);
    printf("Size: %lld\n", ELF_size);
    e.load_ELF(ELF);
    delete[] ELF;
    ELF = nullptr;

    //Initialize window
    is_running = true;
    resize(640, 224);
    setMinimumWidth(640);
    setMinimumHeight(224);
    show();
    return 0;
}

bool EmuWindow::running()
{
    return is_running;
}

void EmuWindow::emulate()
{
    e.run();
    update();
}

void EmuWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    uint32_t* buffer = e.get_framebuffer();
    for (int y = 0; y < 224; y++)
    {
        for (int x = 0; x < 640; x++)
        {
            buffer[x + (y * 640)] |= 0xFF000000;
        }
    }
    QImage image((uint8_t*)buffer, 640, 224, QImage::Format_RGBA8888);

    painter.drawPixmap(0, 0, QPixmap::fromImage(image));
}

void EmuWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
    is_running = false;
}
