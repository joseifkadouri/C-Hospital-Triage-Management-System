#pragma once
#include <string>
#include <vector>
#include <iostream>

struct Patient {
    std::string name;
    int acuity;       // 1 = highest priority
    int arrivalOrder; // lower = earlier

    // New fields (Option B)
    int pain = 0, hr = 0, sbp = 0, spo2 = 0, rr = 0, tempC = 37;
    bool unconscious = false, severeBleeding = false;
    bool chestPain = false, strokeSymptoms = false;

    // Notes
    std::vector<std::string> notes;

    Patient(std::string n = "", int a = 5, int order = 0)
        : name(std::move(n)), acuity(a), arrivalOrder(order) {
    }

    // Comparison for priority: smaller acuity first, then earlier arrival
    bool operator<(const Patient& other) const {
        if (acuity != other.acuity) return acuity < other.acuity;
        return arrivalOrder < other.arrivalOrder;
    }

    void print() const {
        std::cout << name << " (Acuity: " << acuity
            << ", Arrival: " << arrivalOrder << ")";
    }
};
