// main.cpp
#pragma once
#include <QtWidgets>
#include "PriorityQueue.hpp"
#include "TriageAlgorithm.hpp"

// Dialog to view full patient details and manage notes
class PatientDetailsDialog : public QDialog {
    Q_OBJECT
public:
    explicit PatientDetailsDialog(Patient* p, QWidget* parent = nullptr)
        : QDialog(parent), patient_(p)
    {
        setWindowTitle(QString("Patient Details — %1").arg(QString::fromStdString(p->name)));
        auto form = new QFormLayout;

        auto mkLbl = [](const QString& s) {
            auto l = new QLabel(s);
            l->setTextInteractionFlags(Qt::TextSelectableByMouse);
            return l;
            };

        form->addRow("Name:", mkLbl(QString::fromStdString(p->name)));
        form->addRow("Acuity:", mkLbl(QString::number(p->acuity)));
        form->addRow("Arrival:", mkLbl(QString::number(p->arrivalOrder)));
        form->addRow("Pain:", mkLbl(QString::number(p->pain)));
        form->addRow("HR:", mkLbl(QString::number(p->hr)));
        form->addRow("SBP:", mkLbl(QString::number(p->sbp)));
        form->addRow("SpO₂ (%):", mkLbl(QString::number(p->spo2)));
        form->addRow("RR:", mkLbl(QString::number(p->rr)));
        form->addRow("Temp (°C):", mkLbl(QString::number(p->tempC)));
        form->addRow("Unconscious:", mkLbl(p->unconscious ? "Yes" : "No"));
        form->addRow("Severe bleed:", mkLbl(p->severeBleeding ? "Yes" : "No"));
        form->addRow("Chest pain:", mkLbl(p->chestPain ? "Yes" : "No"));
        form->addRow("Stroke sxs:", mkLbl(p->strokeSymptoms ? "Yes" : "No"));

        // Notes UI
        notesList_ = new QListWidget;
        for (auto& n : p->notes) notesList_->addItem(QString::fromStdString(n));
        noteEdit_ = new QLineEdit;
        noteEdit_->setPlaceholderText("Type a note…");
        auto addNoteBtn = new QPushButton("Add Note");
        connect(addNoteBtn, &QPushButton::clicked, this, [this] {
            const QString t = noteEdit_->text().trimmed();
            if (t.isEmpty()) return;
            patient_->notes.push_back(t.toStdString());
            notesList_->addItem(t);
            noteEdit_->clear();
            });

        auto notesBox = new QGroupBox("Notes");
        auto v = new QVBoxLayout;
        v->addWidget(notesList_);
        auto h = new QHBoxLayout;
        h->addWidget(noteEdit_);
        h->addWidget(addNoteBtn);
        v->addLayout(h);
        notesBox->setLayout(v);

        auto dlgLayout = new QVBoxLayout;
        dlgLayout->addLayout(form);
        dlgLayout->addWidget(notesBox);

        auto closeBtn = new QPushButton("Close");
        connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
        dlgLayout->addWidget(closeBtn, 0, Qt::AlignRight);

        setLayout(dlgLayout);
        resize(460, 560);
    }

private:
    Patient* patient_;
    QListWidget* notesList_;
    QLineEdit* noteEdit_;
};

// Main window
class MainWindow : public QWidget {
    Q_OBJECT
public:
    MainWindow() {
        setWindowTitle("Hospital Triage");

        // Inputs
        nameEdit = new QLineEdit;
        painSpin = new QSpinBox;  painSpin->setRange(0, 10);
        hrSpin = new QSpinBox;  hrSpin->setRange(20, 220);   hrSpin->setValue(80);
        sbpSpin = new QSpinBox;  sbpSpin->setRange(50, 220);  sbpSpin->setValue(120);
        spo2Spin = new QSpinBox;  spo2Spin->setRange(50, 100); spo2Spin->setValue(98);
        rrSpin = new QSpinBox;  rrSpin->setRange(5, 60);     rrSpin->setValue(16);    // NEW
        tempSpin = new QSpinBox;  tempSpin->setRange(30, 43);  tempSpin->setValue(37);  // NEW (°C)

        unconsciousBox = new QCheckBox("Unconscious");
        bleedingBox = new QCheckBox("Severe bleeding");
        chestPainBox = new QCheckBox("Chest pain");          // NEW
        strokeBox = new QCheckBox("Stroke symptoms");     // NEW

        auto form = new QFormLayout;
        form->addRow("Name:", nameEdit);
        form->addRow("Pain (0–10):", painSpin);
        form->addRow("Heart Rate:", hrSpin);
        form->addRow("Systolic BP:", sbpSpin);
        form->addRow("SpO₂ (%):", spo2Spin);
        form->addRow("Resp. Rate:", rrSpin);     // NEW
        form->addRow("Temp (°C):", tempSpin);   // NEW
        form->addRow(unconsciousBox);
        form->addRow(bleedingBox);
        form->addRow(chestPainBox);                // NEW
        form->addRow(strokeBox);                   // NEW

        addBtn = new QPushButton("Add Patient");
        peekBtn = new QPushButton("Peek Next");
        treatBtn = new QPushButton("Treat (Dequeue)");
        statusLbl = new QLabel;

        auto btns = new QHBoxLayout;
        btns->addWidget(addBtn);
        btns->addWidget(peekBtn);
        btns->addWidget(treatBtn);

        // Table (now 4 columns with Notes preview)
        table = new QTableWidget(0, 4);
        QStringList headers{ "Name", "Acuity", "Arrival", "Notes" };
        table->setHorizontalHeaderLabels(headers);
        table->horizontalHeader()->setStretchLastSection(true);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setTextElideMode(Qt::ElideRight);
        table->setWordWrap(false);
        table->setContextMenuPolicy(Qt::CustomContextMenu);

        auto layout = new QVBoxLayout;
        layout->addLayout(form);
        layout->addLayout(btns);
        layout->addWidget(table);
        layout->addWidget(statusLbl);
        setLayout(layout);

        connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddPatient);
        connect(peekBtn, &QPushButton::clicked, this, &MainWindow::onPeek);
        connect(treatBtn, &QPushButton::clicked, this, &MainWindow::onTreat);
        connect(table, &QTableWidget::cellDoubleClicked, this, &MainWindow::onRowDoubleClicked);
        connect(table, &QTableWidget::customContextMenuRequested, this, &MainWindow::onTableContextMenu);

        nameEdit->setFocus();
        table->setColumnWidth(3, 280); // nicer width for notes preview
    }

private slots:
    void onAddPatient() {
        const QString name = nameEdit->text().trimmed();
        if (name.isEmpty()) { statusLbl->setText("Enter a name."); return; }

        const int acuity = calculateAcuity2(
            painSpin->value(),
            hrSpin->value(),
            sbpSpin->value(),
            spo2Spin->value(),
            rrSpin->value(),
            tempSpin->value(),
            unconsciousBox->isChecked(),
            bleedingBox->isChecked(),
            chestPainBox->isChecked(),
            strokeBox->isChecked()
        );

        Patient p{ name.toStdString(), acuity, 0 }; // arrivalOrder assigned in queue
        // Store extended fields
        p.pain = painSpin->value();
        p.hr = hrSpin->value();
        p.sbp = sbpSpin->value();
        p.spo2 = spo2Spin->value();
        p.rr = rrSpin->value();
        p.tempC = tempSpin->value();
        p.unconscious = unconsciousBox->isChecked();
        p.severeBleeding = bleedingBox->isChecked();
        p.chestPain = chestPainBox->isChecked();
        p.strokeSymptoms = strokeBox->isChecked();

        queue.enqueue(std::move(p));
        statusLbl->setText(QString("Added %1 (Acuity %2)").arg(name).arg(acuity));
        nameEdit->clear();
        updateTable();
    }

    void onPeek() {
        if (queue.isEmpty()) { statusLbl->setText("Queue empty."); return; }
        Patient p = queue.peek();
        statusLbl->setText(QString("Next: %1 (Acuity %2)")
            .arg(QString::fromStdString(p.name)).arg(p.acuity));
    }

    void onTreat() {
        if (queue.isEmpty()) { statusLbl->setText("Queue empty."); return; }
        Patient p = queue.dequeue();
        statusLbl->setText(QString("Treating: %1 (Acuity %2)")
            .arg(QString::fromStdString(p.name)).arg(p.acuity));
        updateTable();
    }

    void onRowDoubleClicked(int row, int /*col*/) {
        // arrivalOrder in column 2
        bool ok = false;
        int arrival = table->item(row, 2)->text().toInt(&ok);
        if (!ok) return;

        if (auto* pat = queue.findByArrival(arrival)) {
            PatientDetailsDialog dlg(pat, this);
            dlg.exec();        // edits notes in-place
            updateTable();     // refresh notes preview/tooltip
        }
    }

    void onTableContextMenu(const QPoint& pos) {
        const auto idx = table->indexAt(pos);
        if (!idx.isValid()) return;
        int row = idx.row();
        bool ok = false;
        int arrival = table->item(row, 2)->text().toInt(&ok);
        if (!ok) return;
        Patient* pat = queue.findByArrival(arrival);
        if (!pat) return;

        QMenu menu(this);
        QAction* addNote = menu.addAction("Add Note…");
        QAction* viewDet = menu.addAction("View Details…");
        QAction* chosen = menu.exec(table->viewport()->mapToGlobal(pos));
        if (!chosen) return;

        if (chosen == addNote) {
            bool accepted = false;
            QString txt = QInputDialog::getMultiLineText(
                this, "Add Note", "Note:", "", &accepted);
            if (accepted && !txt.trimmed().isEmpty()) {
                pat->notes.push_back(txt.trimmed().toStdString());
                updateTable();
            }
        }
        else if (chosen == viewDet) {
            PatientDetailsDialog dlg(pat, this);
            dlg.exec();
            updateTable();
        }
    }

private:
    // Notes preview helpers
    static QString notePreview(const Patient& p, int maxChars = 60, int maxNotes = 2) {
        if (p.notes.empty()) return "—";
        QStringList ns;
        for (size_t i = 0; i < p.notes.size() && static_cast<int>(i) < maxNotes; ++i)
            ns << QString::fromStdString(p.notes[i]);
        QString joined = ns.join(" • ");
        if (joined.size() > maxChars) joined = joined.left(maxChars - 1) + "…";
        return joined;
    }
    static QString noteTooltip(const Patient& p) {
        if (p.notes.empty()) return "No notes";
        QStringList ns;
        for (auto& n : p.notes) ns << QString::fromStdString(n);
        return ns.join("\n");
    }

    void updateTable() {
        auto items = queue.toVector();
        table->setRowCount(static_cast<int>(items.size()));
        for (int r = 0; r < (int)items.size(); ++r) {
            const auto& p = items[r];

            auto* itName = new QTableWidgetItem(QString::fromStdString(p.name));
            auto* itAcuity = new QTableWidgetItem(QString::number(p.acuity));
            auto* itArr = new QTableWidgetItem(QString::number(p.arrivalOrder));

            QString preview = notePreview(p);
            if (!p.notes.empty())
                preview = QString("(%1) ").arg(p.notes.size()) + preview;

            auto* itNotes = new QTableWidgetItem(preview);
            itNotes->setToolTip(noteTooltip(p));

            table->setItem(r, 0, itName);
            table->setItem(r, 1, itAcuity);
            table->setItem(r, 2, itArr);
            table->setItem(r, 3, itNotes);
        }
        table->resizeColumnsToContents();
        table->setColumnWidth(3, 280);
    }

    // UI controls
    QLineEdit* nameEdit;
    QSpinBox* painSpin, * hrSpin, * sbpSpin, * spo2Spin, * rrSpin, * tempSpin;
    QCheckBox* unconsciousBox, * bleedingBox, * chestPainBox, * strokeBox;
    QPushButton* addBtn, * peekBtn, * treatBtn;
    QTableWidget* table;
    QLabel* statusLbl;

    // Data
    PriorityQueue queue;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.resize(760, 640);
    w.show();
    return app.exec();
}

#include "main.moc"
