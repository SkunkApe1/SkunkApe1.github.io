import java.util.ArrayList;

public class ContactService {
    ArrayList<Contact> contacts;
	
	public ContactService() {
		contacts = new ArrayList<>();
	}
	
	public boolean addContact(Contact newContact) { //creates new contact 
		boolean contains = false;
		for (Contact c : contacts) {
			if (c.getContactID().equalsIgnoreCase(newContact.getContactID())) {
				contains = true;
				break;
			}
		}
		if (!contains) {
			contacts.add(newContact);
			return  true;
			
		}
		else {
			return false;
		}
	}
	
	public boolean deleteContact(String contactID) { //Allows for the deletion of a contact that was previously generated
		boolean deleted = false;
		for (Contact c: contacts) {
			if (c.getContactID(). equalsIgnoreCase(contactID)) {
				contacts.remove(c);
				deleted = true;
				break;
			}
		}
		return deleted;
	}
	
	public boolean updateContactFirstName(String contactID, String newFirstName) { //This is utilized to add a new first name
		boolean updated = false;
		for (Contact c : contacts) {
			if (c.getContactID().equalsIgnoreCase(contactID)) {
				c.setFirstName(newFirstName);
				updated = true;
				break;
			}
		}
		return updated;
	}
	
	public boolean updateContactLastName(String contactID, String newLastName) { //This is utilized to add a new last name
		boolean update = false;
		for (Contact c: contacts) {
			if (c.getContactID().equalsIgnoreCase(contactID)) {
				c.setLastName(newLastName);
				updated = true;
				break;
			}
		}
		return updated;
	}
	public boolean updateContactNumber(String contactID, String newNumber) { //This is utilized to add a new phone number
		boolean update = false;
		for (Contact c : contacts) {
			if (c.getContactID().equalsIgnoreCase(contactID)) {
				c.setPhoneNumber(newNumber);
				updated = true;
				break;
				
			}
		}
		return updated;
	}
	
	public boolean updateContactAddress (String contactID, String newAddress) { //This is utilized to to add a new address
		boolean update = false;
		for (Contact c: contacts) {
			if (c.getContactID().equalsIgnoreCase(contactID)) {
				c.setAddress(newAddress);
				updated = true;
				break;
				
			}
		}
		return updated;
	}
	
	public void displayAllContacts() {
		for(Contact c: contacts) {
			System.out.println(c.toString());
		}
	}
    
}
