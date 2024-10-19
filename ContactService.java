// Graham Swenson - CS499

import java.util.ArrayList;
import java.util.Iterator;

public class ContactService {
    // List to store all contact objects
    private ArrayList<Contact> contacts;

    // Constructor initializes the list of contacts
    public ContactService() {
        contacts = new ArrayList<>();
    }

    // Method to add a new contact
    // Returns true if contact is added, false if contact ID already exists
    public boolean addContact(Contact newContact) {
        if (newContact == null || newContact.getContactID() == null) {
            return false; // Handle null newContact or contactID
        }

        // Check if a contact with the same ID already exists
        boolean contains = false;
        for (Contact c : contacts) {
            if (c.getContactID().equalsIgnoreCase(newContact.getContactID())) {
                contains = true; // Set to true if contactID already exists
                break;
            }
        }

        // If no contact with the same ID, add new contact to the list
        if (!contains) {
            contacts.add(newContact);
            return true; // This is if done successfully
        } else {
            return false; // If Contact ID already in system, returns false
        }
    }

    // Method to delete a contact by contactID
    // Returns true if deletion is successful, false if contact is not found
    public boolean deleteContact(String contactID) {
        if (contactID == null) {
            return false; // Handle null contactID
        }

        boolean deleted = false; // See if deletion occurred
        Iterator<Contact> iterator = contacts.iterator();

        // Use iterator to safely remove elements while iterating
        while (iterator.hasNext()) {
            Contact c = iterator.next();
            if (c.getContactID().equalsIgnoreCase(contactID)) {
                iterator.remove(); // Remove contact safely
                deleted = true;
                break; // Exit the loop after deletion
            }
        }

        return deleted; // Return whether contact was deleted
    }

    // Method to update a contact's first name
    // Returns true if successful, false if contact is not found
    public boolean updateContactFirstName(String contactID, String newFirstName) {
        if (contactID == null || newFirstName == null) {
            return false; // Handle null inputs
        }

        boolean updated = false; // Track if the update occurs

        // Loop through contacts to find the matching contactID
        for (Contact c : contacts) {
            if (c.getContactID().equalsIgnoreCase(contactID)) {
                c.setFirstName(newFirstName); // Update first name
                updated = true;
                break; // Exit the loop after updating
            }
        }
        return updated; // Return whether update was successful
    }

    // Method to update a contact's last name
    public boolean updateContactLastName(String contactID, String newLastName) {
        if (contactID == null || newLastName == null) {
            return false; // Handle null inputs
        }

        boolean updated = false;

        // Loop through contacts to find the matching contactID
        for (Contact c : contacts) {
            if (c.getContactID().equalsIgnoreCase(contactID)) {
                c.setLastName(newLastName); // Update last name
                updated = true;
                break;
            }
        }
        return updated;
    }

    // Method to update a contact's phone number
    public boolean updateContactNumber(String contactID, String newNumber) {
        if (contactID == null || newNumber == null || !newNumber.matches("\\d{10}")) {
            return false; // Handle null or invalid phone number format (e.g., 10 digits)
        }

        boolean updated = false;

        // Loop through contacts to find the matching contactID
        for (Contact c : contacts) {
            if (c.getContactID().equalsIgnoreCase(contactID)) {
                c.setPhoneNumber(newNumber); // Update phone number
                updated = true;
                break;
            }
        }
        return updated;
    }

    // Method to update a contact's address
    public boolean updateContactAddress(String contactID, String newAddress) {
        if (contactID == null || newAddress == null) {
            return false; // Handle null inputs
        }

        boolean updated = false;

        // Loop through contacts to find the matching contactID
        for (Contact c : contacts) {
            if (c.getContactID().equalsIgnoreCase(contactID)) {
                c.setAddress(newAddress); // Update address
                updated = true;
                break;
            }
        }
        return updated;
    }

    // Method to display all contacts
    // Calls the toString() method of each Contact object to display its details
    public void displayAllContacts() {
        for (Contact c : contacts) {
            System.out.println(c.toString());
        }
    }
}

// END CODE