#ifndef __PIM_ATM_H__
#define __PIM_ATM_H__

// --PIM-- from linux/atm.h for some really stupid reason many distributions (fedora) do not put this in their version of linux/atm.h
#ifdef DEFINE_ATMIF
struct atmif_sioc {
    int number;
    int length;
    void *arg;
};
#endif
// --/PIM--

#endif
