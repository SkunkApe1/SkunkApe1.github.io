// Graham Swenson - CS499

import java.util.Optional;

public class Contact {
    private String contactID; //variable for contact id
	private String firstName; // variable for first name
	private String lastName; // variable for last name
	private String number; //variable for phone number
	private Optional<String> address; //variable for address; Optional for null safety
	private Optional<String> email; //variable for email address; Optional for null safety
	
	 // Constructor with validation
    public Contact(String contactID, String firstName, String lastName, String number, String address, String email) {
        // Validate and set contact ID
        if (contactID != null && contactID.length() <= 10) {
            this.contactID = contactID;
        } else {
            throw new IllegalArgumentException("Invalid Contact ID");
        }

        // Validate inputs
        setFirstName(firstName);
        setLastName(lastName);
        setPhoneNumber(number);
        setAddress(address); // Address is optional
        setEmail(email);  // Email is optional
    }

    // First name validation and setter
    public void setFirstName(String firstName) {
        if (firstName != null && firstName.length() <= 10) {
            this.firstName = firstName;
        } else {
            throw new IllegalArgumentException("Invalid first name");
        }
    }

    // Last name validation and setter
    public void setLastName(String lastName) {
        if (lastName != null && lastName.length() <= 10) {
            this.lastName = lastName;
        } else {
            throw new IllegalArgumentException("Invalid last name");
        }
    }

    // Phone number validation and setter
    public void setPhoneNumber(String number) {
        if (number != null && number.matches("\\d{10}")) {  // Validating 10 digits
            this.number = number;
        } else {
            throw new IllegalArgumentException("Invalid phone number");
        }
    }

    // Address validation and setter
    public void setAddress(String address) {
        if (address != null && address.length() <= 30) {
            this.address = Optional.of(address);
        } else {
            this.address = Optional.empty();
        }
    }

    // Email validation and setter
    public void setEmail(String email) {
        if (email != null && email.contains("@") && email.contains(".")) { // common mistakes when entering in an email
            this.email = Optional.of(email);
        } else {
            this.email = Optional.empty(); // Optional to handle potential null email
        }
    }

    // Getters for all attributes
    public String getContactID() {
        return contactID;
    }

    public String getFirstName() {
        return firstName;
    }

    public String getLastName() {
        return lastName;
    }

    public String getPhoneNumber() {
        return number;
    }

    public String getAddress() {
        return address.orElse("No Address Provided"); // Optional fallback if no address entered
    }

    public String getEmail() {
        return email.orElse("No Email Provided"); // Optional fallback if no email entered
    }

    // Override toString() to display all information including email
    @Override
    public String toString() {
        return "Contact [contactID=" + contactID + ", firstName=" + firstName + ", lastName=" + lastName 
            + ", phoneNumber=" + number + ", address=" + address + ", email=" + email + "]";
    }
}

// END CODE