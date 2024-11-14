#pragma once

#include <QMessageBox>

class CitationReminder {
public:
    static void Show() {
        QMessageBox msgBox;
        QString     reminder("VAPOR is developed as an Open Source application by NCAR, ");
        reminder.append("under the sponsorship of the National Science Foundation.\n\n");
        reminder.append("We depend on evidence of the software's value to the scientific community.  ");
        reminder.append("You are free to use VAPOR as permitted under the terms and conditions of the licence.\n\n ");
        reminder.append("Please cite VAPOR in your publications and presentations. ");
        reminder.append("Citation details:\n    https://www.vapor.ucar.edu/pages/vaporCitationPage.html");
        msgBox.setText(reminder);

        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);

        msgBox.exec();
    }
};
