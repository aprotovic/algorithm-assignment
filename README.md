# 🏢 Online Broker Management System (C++ Native GUI)

[![C++](https://img.shields.io/badge/Language-C%2B%2B17-00599C?style=flat&logo=cplusplus&logoColor=white)](https://isocpp.org)
[![GUI](https://img.shields.io/badge/GUI-Win32%20API%20%7C%20GDI%2B-blue.svg)]()
[![Data Structures](https://img.shields.io/badge/DSA-Custom%20Implementations-orange.svg)]()
[![Coursework](https://img.shields.io/badge/Course-SEng2022%20%40%20DBU-purple.svg)]()

A high-performance desktop property and vehicle brokerage platform built natively in **C++** using the **Win32 API and GDI+**. Engineered as a capstone project for *Design and Analysis of Data Structures and Algorithms (SEng2022)* at Debre Berhan University.

---

## 🧠 Custom Data Structures (Built From Scratch)

To strictly satisfy computer science rigor, **all data structures were implemented without the C++ STL**:

- **Doubly Linked List (`LinkedList.h`):** Primary dynamic container for `House` and `Car` objects, enabling efficient $O(1)$ insertions and node manipulation without array reallocation.
- **Hash Table with Chaining (`HashTable.h`):** $O(1)$ average-time lookup converting UI listing identifiers directly into listing pointers.
- **Binary Search Tree (`BST.h`):** In-order traversal to extract and display property listings in sorted price order.
- **Queue (`Queue.h`):** First-In, First-Out (FIFO) queue for fair order and transaction processing.
- **Sorting Algorithms (`Algorithms.h`):** Custom sorting algorithm to reorder linked lists dynamically by pricing criteria.

---

## ✨ System Capabilities

- **Role-Based Controls:** Distinct interfaces for **Admins**, **Verified Sellers**, and **Buyers**.
- **Account KYC Verification:** Uploaded identity documents must be approved by the admin prior to marketplace trading privileges.
- **Native Windows GUI:** Custom-drawn interface leveraging native Win32 controls, file dialogs, and GDI+ graphic rendering.
- **Financial Calculation:** Automatic 5% broker commission calculation upon sale approval and local payment logging.
- **Dual Persistence:** Automatic text database persistence (`database.txt`, `users.txt`, `orders.txt`) alongside an included relational MySQL schema (`schema.sql`).

---

## 🚀 Build & Run (Windows)

Compile directly with MinGW / GCC (`g++`):

```bash
# Clone repository
git clone https://github.com/aprotovic/algorithm-assignment.git
cd algorithm-assignment

# Build using the included batch script (links GDI+, ComDlg32, Shell32)
build.bat

# Launch the desktop executable
./BrokerSystem.exe
```

---

## 👥 Authors
Developed by Debre Berhan University Software Engineering Students (Goitom Shumey, Ezra Behailu, Fiker Mamo, Firdos Abdu, Hosaena Anteneh).
