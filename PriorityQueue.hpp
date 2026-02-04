#pragma once
#include "Patient.hpp"
#include <iostream>
#include <vector>

class PriorityQueue 
{
private:
    struct Node 
    {
        Patient data;
        Node* next;
        explicit Node(Patient p) : data(std::move(p)), next(nullptr) {}
    };

    Node* front_ = nullptr;
    Node* rear_ = nullptr;
    int   arrivalCounter_ = 0;

public:
    PriorityQueue() = default;
    ~PriorityQueue() 
    { 
        while (!isEmpty()) 
            dequeue(); 
    }

    bool isEmpty() const 
    { 
        return front_ == nullptr; 
    }

    // Existing: build Patient from (name, acuity)
    void enqueue(const std::string& name, int acuity) 
    {
        Patient p{ name, acuity, arrivalCounter_++ };
        enqueue(std::move(p));
    }

    // NEW: enqueue a fully-populated Patient (Option B)
    void enqueue(Patient&& p) 
    {
        // ensure arrivalOrder is set if caller didn’t
        if (p.arrivalOrder == 0 && (isEmpty() || p.arrivalOrder == 0))
            p.arrivalOrder = arrivalCounter_++;
        Node* n = new Node(std::move(p));

        if (isEmpty()) 
        {
            front_ = rear_ = n; 
            return; 
        }

        if (n->data < front_->data) 
        {
            n->next = front_; 
            front_ = n;
            return; 
        }

        if (!(n->data < rear_->data)) 
        { 
            rear_->next = n; 
            rear_ = n; 
            return; 
        }

        Node* cur = front_;
        while (cur->next && !(n->data < cur->next->data)) cur = cur->next;
        n->next = cur->next;
        cur->next = n;
    }

    Patient dequeue() 
    {
        if (isEmpty()) 
        {
            std::cout << "Queue is empty.\n"; return Patient();
        }
        Node* tmp = front_;
        Patient out = tmp->data;
        front_ = front_->next;
        if (!front_) rear_ = nullptr;
        delete tmp;
        return out;
    }

    Patient peek() const 
    {
        if (isEmpty()) 
        { 
            std::cout << "Queue is empty.\n"; return Patient(); 
        }
        return front_->data;
    }

    std::vector<Patient> toVector() const 
    {
        std::vector<Patient> v;
        for (Node* cur = front_; cur; cur = cur->next) v.push_back(cur->data);
        return v;
    }

    // NEW: find a patient by arrivalOrder (for editing notes)
    Patient* findByArrival(int arrivalOrder) 
    {
        for (Node* cur = front_; cur; cur = cur->next) 
        {
            if (cur->data.arrivalOrder == arrivalOrder) return &cur->data;
        }
        return nullptr;
    }
};
