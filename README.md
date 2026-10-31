# Hotel Management System (Case Study) - C++ OOP

**Name:** Kairava Rora
**UID:** 25BCD10071
**Section:** 25BCD-2-A
**Subject:** Object Oriented Programming (C++)
**Submitted to:** Jitendra Kumar, Assistant Professor

## About the case study
A case study on the design and implementation of a Hotel Management System using
Object Oriented Programming in C++. It covers requirement analysis, class design,
mapping of OOP concepts, implementation, testing and evaluation against design principles.

## Features
- Room hierarchy: Standard, Deluxe and Suite (different rate and guest capacity)
- Guest hierarchy: Regular, Silver (5% discount) and Gold (10% discount)
- Booking with double-booking and capacity checks
- Extra services (room service, laundry) added to a booking using `operator+=`
- Itemised bill with discount and 12% GST at check-out
- Occupancy report
- Custom exceptions: `HotelException` and `RoomUnavailableException`

## OOP concepts used
- **Encapsulation:** protected/private data, controlled access through member functions
- **Abstraction:** abstract class `Room` with pure virtual functions
- **Inheritance:** `StandardRoom`, `DeluxeRoom`, `SuiteRoom`; `SilverMember`, `GoldMember`
- **Polymorphism:** virtual `ratePerNight()`, `maxGuests()`, `discountRate()`
- **Operator overloading and friend function:** `operator<<` for `Room`, `operator+=` for `Booking`
- **Static member:** `Booking::totalBookings`
- **Exception handling, STL, smart pointers and a function template**

## How to compile and run
```
g++ -std=c++14 hotel_case_study.cpp -o hotel_case
./hotel_case
```
On Windows (MinGW): `g++ -std=c++14 hotel_case_study.cpp -o hotel_case.exe` then run `hotel_case.exe`

## Sample output
See `sample_output.txt`.

## Files
- `hotel_case_study.cpp` - complete source code
- `README.md` - this file
- `sample_output.txt` - output of the program
- `Case_Study_Report_25BCD10071.pdf` - case study report (add your PDF here)
