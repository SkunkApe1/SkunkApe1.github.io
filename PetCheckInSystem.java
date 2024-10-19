// Graham Swenson - CS499

import java.time.LocalDateTime;
import java.time.Month;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

class Pet {
    // Attributes for pet
    String petType; // Type of pet
    String petName; // Name of pet
    int petAge; // Age of the pet
    int dogSpaces = 30; // Total available spaces for dogs - set to 30
    int catSpaces = 15; // Total available spaces for cats - set to 15
    int daysStay; // Number of days at the pet hotel
    double amountDue; // Total cost due for the stay
    double dogWeight; // Weight of the dog
    boolean groomingAdded; // Track if grooming was added
    LocalDateTime checkInDateTime; // Date and time of check-in

    // Store pet check-in history for data mining
    static List<Pet> petHistory = new ArrayList<>();

    // Main check-in method
    public void checkIn() {
        Scanner sc = new Scanner(System.in); // Scanner for the user's input

        // Record the date and time of check-in
        checkInDateTime = LocalDateTime.now();
        DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");
        System.out.println("Check-in Date and Time: " + checkInDateTime.format(formatter));

        // Intro message
        System.out.println("Welcome to the Automated Pet Check-in (APC)");
        System.out.println("What type of animal are you checking in? (Dog/Cat)");
        petType = sc.nextLine().toLowerCase(); // Get the pet type from the user

        // If the pet is a dog
        if (petType.equals("dog")) {
            if (dogSpaces > 0) {
                System.out.println("There are " + dogSpaces + " dog spaces available.");
            } else {
                System.out.println("Sorry, no dog vacancy");
                return; // Exit if no vacancy
            }
        }

        // If the pet is a cat
        else if (petType.equals("cat")) {
            if (catSpaces > 0) {
                System.out.println("There are " + catSpaces + " cat spaces available.");
            } else { 
                System.out.println("Sorry, no cat vacancy");
                return; // Exit if no vacancy
            }
        }
        // Invalid input for animal type
        else {
            System.out.println("Invalid animal type");
            return; // Bad input - return
        }

        // Check if the animal has been in the system before
        System.out.println("Has this pet been checked in before? (Y/N)");
        String hasCheckedInBefore = sc.nextLine().toUpperCase(); // Convert to uppercase for easier comparison

        // If pet has not been here
        if (hasCheckedInBefore.equals("N")) {
            getPetInfo(sc); // Call method to gather new pet information
        }

        // If pet HAS been here
        else if (hasCheckedInBefore.equals("Y")) {
            System.out.println("Please verify the info is up to date.");
            getPetInfo(sc); // Currently still getting the pet info
        }

        // Ask for the number of days the pet will stay
        System.out.println("How many days will your pet(s) stay?");
        daysStay = sc.nextInt(); // Store number of days

        // If the pet is a dog and is staying for 2 or more days, offer grooming
        if (petType.equals("dog") && daysStay >= 2) {
            System.out.println("Would you like to add grooming for your dog? (Y/N)");
            sc.nextLine(); // Consume the leftover newline character
            String grooming = sc.nextLine().toUpperCase(); // Get response for grooming
            if (grooming.equals("Y")) {
                System.out.println("Grooming added.");
                amountDue += 50; // Add grooming fee set at $50 USD
                groomingAdded = true;
            }
        }

        // Confirm the user's selections
        confirmSelection();

        // Assign the pet to a space (dog or cat)
        assignSpace();

        // Add to pet history for data mining
        petHistory.add(this);
    }

    // Method to gather or verify pet info
    public void getPetInfo(Scanner sc) {
        // Ask for the pet's name
        System.out.println("Enter pet's name:");
        petName = sc.nextLine(); // Store pet's name

        // Ask for the pet's age
        System.out.println("Enter pet's age:");
        petAge = sc.nextInt(); // Store pet's age

        // If the pet is a dog, ask for the dog's weight
        if (petType.equals("dog")) {
            System.out.println("Enter dog's weight:");
            dogWeight = sc.nextDouble(); // Store dog's weight
        }
        sc.nextLine(); // Consume the leftover newline character
    }

    // Method to confirm the user's selections
    public void confirmSelection() {
        System.out.println("Confirming your selections...");
        System.out.println("Pet Type: " + petType);  // Confirm pet type
        System.out.println("Pet Name: " + petName);  // Confirm pet name
        System.out.println("Pet Age: " + petAge);    // Confirm pet age
        System.out.println("Days Stay: " + daysStay); // Confirm number of stay days

        // If the pet is a dog, confirm the dog's weight
        if (petType.equals("dog")) {
            System.out.println("Dog Weight: " + dogWeight);
        }
    }

    // Method to assign the pet to a space (either dog or cat space)
    public void assignSpace() {
        // If the pet is a dog, reduce available dog spaces
        if (petType.equals("dog")) {
            dogSpaces--; // Decrease available dog spaces by 1
            System.out.println("SUCCESS: Dog assigned to a space... Remaining dog spaces: " + dogSpaces);
        } 
        // If the pet is a cat, reduce available cat spaces
        else if (petType.equals("cat")) {
            catSpaces--; // Decrease available cat spaces by 1
            System.out.println("SUCCESS: Cat assigned to a space... Remaining cat spaces: " + catSpaces);
        }
    }

    // Data mining methods
    public static void dataMining() {
        System.out.println("\n--- Data Mining Report ---");

        // Count occurrences of each pet type
        int dogCount = 0, catCount = 0;
        int groomingCount = 0;
        double totalStay = 0;
        int[] monthCheckIn = new int[12]; // Array to track check-ins by month

        for (Pet pet : petHistory) {
            if (pet.petType.equals("dog")) dogCount++;
            if (pet.petType.equals("cat")) catCount++;
            if (pet.groomingAdded) groomingCount++;
            totalStay += pet.daysStay;

            // Increment the corresponding month
            monthCheckIn[pet.checkInDateTime.getMonthValue() - 1]++;
        }

        int totalPets = dogCount + catCount;

        System.out.println("Total Pets Checked In: " + totalPets);
        System.out.println("Dogs Checked In: " + dogCount);
        System.out.println("Cats Checked In: " + catCount);
        System.out.println("Grooming Services Opted: " + groomingCount);
        System.out.println("Average Days of Stay: " + (totalPets > 0 ? totalStay / totalPets : 0));

        // Display the busiest month for check-ins
        int busiestMonth = 0;
        for (int i = 1; i < monthCheckIn.length; i++) {
            if (monthCheckIn[i] > monthCheckIn[busiestMonth]) {
                busiestMonth = i;
            }
        }
        System.out.println("Busiest Check-in Month: " + Month.of(busiestMonth + 1));
    }
}

// Main class to run the Pet Check-in system
public class PetCheckInSystem {
    public static void main(String[] args) {
        Pet pet = new Pet(); // Create a new pet object
        pet.checkIn();       // Start the check-in process
        
        // Display data mining report
        Pet.dataMining();
    }
}

//End Code