#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <cmath>
using namespace std;

// ---------- User-defined exceptions ----------
class HotelException : public runtime_error {
public:
    explicit HotelException(const string& m) : runtime_error(m) {}
};
class RoomUnavailableException : public HotelException {
public:
    explicit RoomUnavailableException(const string& m) : HotelException(m) {}
};

// ---------- Service ----------
struct Service {
    string name;
    double price;
};

// ---------- Room hierarchy ----------
class Room {
protected:
    int roomNo;
    bool occupied;
public:
    explicit Room(int n) : roomNo(n), occupied(false) {}
    virtual ~Room() {}
    virtual string type() const = 0;
    virtual double ratePerNight() const = 0;
    virtual int maxGuests() const = 0;
    int getRoomNo() const { return roomNo; }
    bool isOccupied() const { return occupied; }
    void setOccupied(bool o) { occupied = o; }

    friend ostream& operator<<(ostream& os, const Room& r) {
        os << "Room " << r.roomNo << " | " << r.type() << " | Rs. "
           << fixed << setprecision(2) << r.ratePerNight()
           << " | Max guests " << r.maxGuests() << " | "
           << (r.occupied ? "Occupied" : "Vacant");
        return os;
    }
};

class StandardRoom : public Room {
public:
    using Room::Room;
    string type() const override { return "Standard"; }
    double ratePerNight() const override { return 2000.0; }
    int maxGuests() const override { return 2; }
};

class DeluxeRoom : public Room {
public:
    using Room::Room;
    string type() const override { return "Deluxe"; }
    double ratePerNight() const override { return 3500.0; }
    int maxGuests() const override { return 3; }
};

class SuiteRoom : public Room {
public:
    using Room::Room;
    string type() const override { return "Suite"; }
    double ratePerNight() const override { return 6000.0; }
    int maxGuests() const override { return 5; }
};

// ---------- Guest hierarchy ----------
class Guest {
protected:
    int id;
    string name;
public:
    Guest(int i, string n) : id(i), name(move(n)) {}
    virtual ~Guest() {}
    virtual string category() const { return "Regular"; }
    virtual double discountRate() const { return 0.0; }
    int getId() const { return id; }
    string getName() const { return name; }
};

class SilverMember : public Guest {
public:
    using Guest::Guest;
    string category() const override { return "Silver"; }
    double discountRate() const override { return 0.05; }
};

class GoldMember : public Guest {
public:
    using Guest::Guest;
    string category() const override { return "Gold"; }
    double discountRate() const override { return 0.10; }
};

// ---------- Booking ----------
class Booking {
    int id, guestId, roomNo, nights;
    double rate;
    bool active;
    vector<Service> services;
    static int totalBookings;
public:
    Booking(int i, int g, int r, int n, double rt)
        : id(i), guestId(g), roomNo(r), nights(n), rate(rt), active(true) {
        ++totalBookings;
    }
    Booking& operator+=(const Service& s) { services.push_back(s); return *this; }

    double roomCharge() const { return rate * nights; }
    double serviceCharge() const {
        double t = 0;
        for (const auto& s : services) t += s.price;
        return t;
    }
    double subtotal() const { return roomCharge() + serviceCharge(); }

    int getId() const { return id; }
    int getGuestId() const { return guestId; }
    int getRoomNo() const { return roomNo; }
    int getNights() const { return nights; }
    double getRate() const { return rate; }
    bool isActive() const { return active; }
    const vector<Service>& getServices() const { return services; }
    void close() { active = false; }
    static int getTotal() { return totalBookings; }
};
int Booking::totalBookings = 0;

// ---------- Hotel ----------
class Hotel {
    string name;
    vector<unique_ptr<Room>> rooms;
    vector<unique_ptr<Guest>> guests;
    vector<Booking> bookings;
    int nextId = 1;

    Room& findRoom(int no) {
        for (auto& r : rooms) if (r->getRoomNo() == no) return *r;
        throw runtime_error("Room " + to_string(no) + " not found.");
    }
    Guest& findGuest(int id) {
        for (auto& g : guests) if (g->getId() == id) return *g;
        throw runtime_error("Guest " + to_string(id) + " not found.");
    }
    Booking& findBooking(int id) {
        for (auto& b : bookings) if (b.getId() == id) return b;
        throw runtime_error("Booking " + to_string(id) + " not found.");
    }
public:
    explicit Hotel(string n) : name(move(n)) {}

    void addRoom(unique_ptr<Room> r) { rooms.push_back(move(r)); }
    void addGuest(unique_ptr<Guest> g) { guests.push_back(move(g)); }

    void book(int guestId, int roomNo, int nights, int persons) {
        if (nights <= 0)
            throw invalid_argument("Number of nights must be positive.");
        Guest& g = findGuest(guestId);
        Room& r = findRoom(roomNo);
        if (r.isOccupied())
            throw RoomUnavailableException(
                "Room " + to_string(roomNo) + " is already occupied.");
        if (persons > r.maxGuests())
            throw HotelException("Room " + to_string(roomNo) + " (" + r.type() +
                ") allows at most " + to_string(r.maxGuests()) + " guests.");
        r.setOccupied(true);
        int id = nextId++;
        bookings.emplace_back(id, guestId, roomNo, nights, r.ratePerNight());
        cout << "  Booking " << id << " confirmed for " << g.getName()
             << " in room " << roomNo << ".\n";
    }

    void addService(int bookingId, const string& sName, double price) {
        Booking& b = findBooking(bookingId);
        if (!b.isActive())
            throw HotelException("Booking " + to_string(bookingId) + " is already closed.");
        if (price <= 0)
            throw invalid_argument("Service price must be positive.");
        b += Service{sName, price};
        cout << "  Added '" << sName << "' to booking " << bookingId << ".\n";
    }

    void checkout(int bookingId) {
        Booking& b = findBooking(bookingId);
        if (!b.isActive())
            throw HotelException("Booking " + to_string(bookingId) + " is already closed.");
        Guest& g = findGuest(b.getGuestId());
        Room& r = findRoom(b.getRoomNo());

        double subtotal = b.subtotal();
        double discount = subtotal * g.discountRate();   // polymorphic call
        double taxable  = subtotal - discount;
        const double GST = 0.12;
        double gst   = taxable * GST;
        double total = taxable + gst;

        cout << fixed << setprecision(2);
        cout << "  ========== BILL ==========\n";
        cout << "  Booking   : " << bookingId << "\n";
        cout << "  Guest     : " << g.getName() << " (" << g.category() << ")\n";
        cout << "  Room      : " << r.getRoomNo() << " (" << r.type() << ")\n";
        cout << "  Stay      : " << b.getNights() << " night(s) x Rs. "
             << b.getRate() << " = Rs. " << b.roomCharge() << "\n";
        if (b.getServices().empty()) {
            cout << "  Services  : none\n";
        } else {
            cout << "  Services  :\n";
            for (const auto& s : b.getServices())
                cout << "    - " << s.name << " : Rs. " << s.price << "\n";
        }
        cout << "  Subtotal  : Rs. " << subtotal << "\n";
        cout << "  Discount  : Rs. " << discount << " ("
             << lround(g.discountRate() * 100) << "%)\n";
        cout << "  GST (12%) : Rs. " << gst << "\n";
        cout << "  TOTAL     : Rs. " << total << "\n";

        r.setOccupied(false);
        b.close();
    }

    void report() const {
        int occ = 0;
        for (const auto& r : rooms) if (r->isOccupied()) ++occ;
        double pct = rooms.empty() ? 0.0 : 100.0 * occ / rooms.size();
        cout << fixed << setprecision(2);
        cout << "  " << name << " - Occupancy: " << occ << " of "
             << rooms.size() << " rooms (" << pct << "%)\n";
        for (const auto& r : rooms) cout << "  " << *r << "\n";
    }
};

// ---------- Helper to run one operation safely ----------
template <typename Func>
void attempt(const string& label, Func f) {
    cout << "> " << label << "\n";
    try {
        f();
        cout << "  OK\n";
    } catch (const RoomUnavailableException& e) {
        cout << "  Room unavailable: " << e.what() << "\n";
    } catch (const HotelException& e) {
        cout << "  Booking rule error: " << e.what() << "\n";
    } catch (const exception& e) {
        cout << "  Error: " << e.what() << "\n";
    }
}

// ---------- Main ----------
int main() {
    Hotel hotel("Grand Residency");
    hotel.addRoom(make_unique<StandardRoom>(101));
    hotel.addRoom(make_unique<StandardRoom>(102));
    hotel.addRoom(make_unique<DeluxeRoom>(201));
    hotel.addRoom(make_unique<SuiteRoom>(301));
    hotel.addGuest(make_unique<Guest>(1, "Kairava Rora"));
    hotel.addGuest(make_unique<GoldMember>(2, "Riya Sharma"));
    hotel.addGuest(make_unique<SilverMember>(3, "Arjun Singh"));

    attempt("Book room 101 for guest 1 (2 nights, 2 persons)",
            [&] { hotel.book(1, 101, 2, 2); });
    attempt("Book room 101 for guest 2 (already occupied)",
            [&] { hotel.book(2, 101, 1, 1); });
    attempt("Book room 102 for guest 3 (4 persons)",
            [&] { hotel.book(3, 102, 1, 4); });
    attempt("Book suite 301 for guest 2 (3 nights, 4 persons)",
            [&] { hotel.book(2, 301, 3, 4); });
    attempt("Add dinner to booking 2",
            [&] { hotel.addService(2, "Room Service Dinner", 1800); });
    attempt("Add laundry to booking 2",
            [&] { hotel.addService(2, "Laundry", 600); });
    attempt("Occupancy report", [&] { hotel.report(); });
    attempt("Check out booking 1", [&] { hotel.checkout(1); });
    attempt("Check out booking 2", [&] { hotel.checkout(2); });
    attempt("Check out booking 1 again", [&] { hotel.checkout(1); });
    attempt("Book room 201 for unknown guest 9",
            [&] { hotel.book(9, 201, 1, 1); });
    attempt("Book room 201 for guest 3 for 0 nights",
            [&] { hotel.book(3, 201, 0, 1); });

    cout << "\nTotal bookings created: " << Booking::getTotal() << "\n";
    return 0;
}
