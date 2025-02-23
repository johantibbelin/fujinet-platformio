#ifndef _PRINTERLIST_H
#define _PRINTERLIST_H

#include "printer.h"

#define PRINTERLIST_SIZE 4
class printerlist
{
private:
    struct printerlist_entry
    {
        ACSIPrinter::printer_type type = ACSIPrinter::printer_type::PRINTER_INVALID;
        ACSIPrinter *pPrinter = nullptr;
        int port = 0;
    };
    printerlist_entry _printers[PRINTERLIST_SIZE];

public:
    void set_entry(int index, ACSIPrinter *ptr, ACSIPrinter::printer_type ptype, int pport);

    void set_ptr(int index, ACSIPrinter *ptr);
    void set_type(int index, ACSIPrinter::printer_type ptype);
    void set_port(int index, int pport);

    ACSIPrinter * get_ptr(int index);
    ACSIPrinter::printer_type get_type(int index);
    int get_port(int index);
};

extern printerlist fnPrinters;

#endif // _PRINTERLIST_H