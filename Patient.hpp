#pragma once
#include <string>
#include <vector>
#include <iostream>

using namespace std;

struct Patient {
    string name;
    int acuity;       // 1 = highest priority
    int arrivalOrder; // lower = earlier

    int pain = 0, hr = 0, sbp = 0, spo2 = 0, rr = 0, tempC = 37;
    bool unconscious = false, severeBleeding = false;
    bool chestPain = false, strokeSymptoms = false;

    vector<string> notes;

    Patient(string n = "", int a = 5, int order = 0) : name(move(n)), acuity(a), arrivalOrder(order) {}

    //Comparison for priority: smaller acuity first, then earlier arrival
    bool operator<(const Patient& other) const 
    {
        if (acuity != other.acuity) return acuity < other.acuity;
        return arrivalOrder < other.arrivalOrder;
    }

    void print() const 
    {
        cout << name << " (Acuity: " << acuity << ", Arrival: " << arrivalOrder << ")";
    }
};

