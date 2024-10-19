// Graham Swenson - CS499

public class Contact {
    private String contactID; //variable for contact id
	private String firstName; // variable for first name
	private String lastName; // variable for last name
	private String Number; //variable for phone number
	private String Address; //variable for address
	
	public Contact(String contactIdent, String fName, String lName, String Num, String Add){
		if(contactIdent.length() <= 10 && contactIdent != null) {
			this.contactID = contactIdent;
		}
		this.inputFirstName(fName);
		this.inputLastName(lName);
		this.inputPhoneNumber(Num);
		this.inputAddress(Add);
	
	}
	public void inputFirstName(String fName) {
		if(fName.length() <= 10 && fName != null) {
			this.firstName = fName;
		}
	}
	public void inputLastName(String lName) {
		if(lName.length() <= 10 && lName != null) {
			this.lastName = lName;
		}
	}
	public void inputPhoneNumber(String Number) {
		if(Number.length() == 10 && Number != null) {
			this.Number = Number;
		}
	}
	public void inputAddress(String Address) {
		if(Address.length() <= 30 && Address != null){
			this.Address = Address;
		}
	}
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
		return Number;
	}
	public String getAddress() {
		return Address;
	}
	
	@Override
	public String toString() {
		return "Contact [contactID=" + contactID + ", firstName =" +firstName + ", lastName=" + lastName + ", phoneNumber=" + Number + ", address=" + Address + "]";
	}
    
}
