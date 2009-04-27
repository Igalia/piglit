//
// Author:    Philip Rideout
// Copyright: 2002-2005  3Dlabs Inc. Ltd.  All rights reserved.
// License:   see 3Dlabs-license.txt
//

#ifndef TABLES_H
#define TABLES_H

struct Id {
    enum EId {
        FileSave = 1,
        FileExit,

        HelpCommandLine,
        HelpAbout,

        WidgetHtml,
        WidgetCanvas,
    };

    union {
        EId name;
        int value;
    };

    Id(EId name) : name(name) {}
    Id(int value) : value(value) {}
    Id() : value(0) {}
    operator EId() const { return name; }
    void operator++() { ++value; }
    void operator++(int) { value++; }
};

#endif
