#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/random.h>
#include <linux/list.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Adds random passengers to a list and uses proc to query the stats");

#define ENTRY_NAME "passenger_list"
#define ENTRY_SIZE 1000
#define PERMS 0644
#define PARENT NULL

static struct proc_ops fops;

extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int start_floor, int destination_floor, int type);
extern int (*STUB_stop_elevator)(void);


static char *message;


static int elevator_state = OFFLINE;
static int current_floor = 1;
static int passengers_serviced = 0;
static int floor_count = 0; //for all floors total
static int read_p;


#define FRESHMAN 0
#define SOPHOMORE 1
#define JUNIOR 2
#define SENIOR 3

#define OFFLINE 0
#define IDLE 1
#define LOADING 2
#define UP 3
#define DOWN 4

#define NUM_PASSENGER_TYPES 4
#define MAX_PASSENGERS 5



struct Elevator{ //linked list of passengers on elevator & total weight and count
	int total_cnt;
	int total_weight;
	struct list_head list;
} ;



typedef struct passenger { //information of each passenger
	char id[3];
	int weight;
	int destination;
	struct list_head list;
} Passenger; //Passenger can be used as nickname

Passenger f1, f2, f3, f4, f5, f6; //one for each floor
Elevator elevator;

/********************************************************************/

int issue_request(int start, int dest, int type) { //adds passengers to floors
	//create lock here????
	int weight;
	char id[2] = ""; //id is placeholder for p->id before it has memory allocated
	Passenger *p == NULL;

	switch (type) {
		case FRESHMAN:
			weight = 100;
			strcpy(id, "F");
			break;
		case SOPHOMORE:
			weight = 150;
			strcpy(id, "O");
			break;
		case JUNIOR:
			weight = 200;
			strcpy(id, "J");
			break;
		case SENIOR:
			weight = 250;
			strcpy(id, "S");
			break;
		default:
			return 1; //returns 1 if invalid
	}

	p = kmalloc(sizeof(Passenger) * 1, __GFP_RECLAIM); //allocate memory for p
	if (p == NULL)
		return -ENOMEM; // return 1 for any invalid request for issue_request but error is -ENOMEM?

	strcpy(p->id, id); //J, O, S, ...
	strcat(p->id, dest); //J1, O3, S5...
	p->weight = weight;
	p->destination = dest;

	switch (start) {
		case 1:
			list_add_tail(&p->list, &f1.list); //adds to floor 1 waiting list
			break;
		case 2:
			list_add_tail(&p->list, &f2.list); //adds to floor 2 waiting list
			break;
		case 3:
			list_add_tail(&p->list, &f3.list); //adds to floor 3 waiting list
			break;
		case 4:
			list_add_tail(&p->list, &f4.list); //adds to floor 4 waiting list
			break;
		case 5:
			list_add_tail(&p->list, &f5.list); //adds to floor 5 waiting list
			break;
		case 6:
			list_add_tail(&p->list, &f6.list); //adds to floor 6 waiting list
			break;
		default:
			return 1; //returns 1 if invalid
	}
	floor_count++;
	//lock release here????
	return 0; //success
/*
	if (passengers.total_cnt + 1 > MAX_PASSENGERS || passengers.total_weight + weight > 750)
		return 0; // does not add to list

	//list_add(&a->list, &animals.list); // insert at front of list

	
	list_add_tail(&p->list, &passengers.list); //adds passenger to back of elevator list
	Passenger *last_element = list_last_entry(&Passenger.list, Passenger, list);
	list_del(&last_element->list);//removes   passenger from floor waiting list
	passengers.total_cnt += 1;
	passengers.total_weight += weight;*/
}

int addToElevator(int currentFloor) { //loads to elevator
	Passenger * p;
	struct list_head *temp;
	struct list_head *tempSafe;

	switch(currentFloor) {
		case 1:
			list_for_each_safe(temp, tempSafe, &f1.list) { //use safe version because we will be deleting in iterations
				p = list_entry(temp, Passenger, list);
				if (elevator.total_weight + p->weight <= 750 && elevator.total_cnt < 5)	{
					list_add_tail(&p->list, &elevator.list);
					elevator.total_weight += p->weight;
					elevator.total_cnt++;
					floor_count--;
					list_del(temp); //remove current passenger (pointed by temp) from floor list
				}
			}
			break;
		case 2:
			list_for_each_safe(temp, tempSafe, &f2.list) { /* forwards*/
				p = list_entry(temp, Passenger, list);
				if (elevator.total_weight + p->weight <= 750 && elevator.total_cnt < 5)	{
					list_add_tail(&p->list, &elevator.list);
					elevator.total_weight += p->weight;
					elevator.total_cnt++;
					floor_count--;
					list_del(temp);
				}
			}
			break;
		case 3:
			list_for_each_safe(temp, tempSafe, &f3.list) { /* forwards*/
				p = list_entry(temp, Passenger, list);
				if (elevator.total_weight + p->weight <= 750 && elevator.total_cnt < 5)	{
					list_add_tail(&p->list, &elevator.list);
					elevator.total_weight += p->weight;
					elevator.total_cnt++;
					floor_count--;
					list_del(temp);
				}
			}
			break;
		case 4:
			list_for_each_safe(temp, tempSafe, &f4.list) { /* forwards*/
				p = list_entry(temp, Passenger, list);
				if (elevator.total_weight + p->weight <= 750 && elevator.total_cnt < 5)	{
					list_add_tail(&p->list, &elevator.list);
					elevator.total_weight += p->weight;
					elevator.total_cnt++;
					floor_count--;
					list_del(temp);
				}
			}
			break;
		case 5:
			list_for_each_safe(temp, tempSafe, &f5.list) { /* forwards*/
				p = list_entry(temp, Passenger, list);
				if (elevator.total_weight + p->weight <= 750 && elevator.total_cnt < 5)	{
					list_add_tail(&p->list, &elevator.list);
					elevator.total_weight += p->weight;
					elevator.total_cnt++;
					floor_count--;
					list_del(temp);
				}
			}
			break;
		case 6:
			list_for_each_safe(temp, tempSafe, &f6.list) { /* forwards*/
				p = list_entry(temp, Passenger, list);
				if (elevator.total_weight + p->weight <= 750 && elevator.total_cnt < 5)	{
					list_add_tail(&p->list, &elevator.list);
					elevator.total_weight += p->weight;
					elevator.total_cnt++;
					floor_count--;
					list_del(temp);
				}
			}
			break;
	}
	return 0;
}

int removeFromElevator() { //unloads from elevator from current floor USED BY KTHREAD MOVEMENT MAYBE???
	Passenger * checkFirst; //unloads the person who was in the elevator the longest
	Passenger * p;
	struct list_head *elevatorPtr = &elevator.list;
	struct list_head *safePtr;

	if (!list_empty(elevatorPtr))  //if elevator isn't empty (statement might not be necessary)
 		checkFirst = list_first_entry(elevatorPtr, Passenger, list);
	else
		return 1; //error can't remove bc empty

	//if current floor is proper destination for first passenger, unload each passenger with same destination
	if (checkFirst->destination == current_floor) {
		list_for_each_safe(elevatorPtr, safePtr, &elevator.list) {
			p = list_entry(elevatorPtr, Passenger, list);
			if (p->destination == current_floor) {
				list_del(elevatorPtr); //remove current entry of elevator from list
				kfree(p); //deallocate memory tied to p as it's no longer needed once unloaded
			}
		}
	}
	else
		return -1; //error not correct floor (can be used by kthread to check when to stop moving)

	return 0; //successful unloading

}

int printElevator() {
	Passenger * p;
	struct list_head *temp;
	char *buf = kmalloc(sizeof(char) * 100, __GFP_RECLAIM);

	list_for_each(temp, &elevator.list) {
		p = list_entry(temp, Passenger, list);
		sprintf(buf, "%s ", p->id);
		strcat(message, buf);
	}
	strcat(message, "\n");
	kfree(buf);
	return 0;
}

int printFloorList(int floorNum) {
	Passenger *p;
	struct list_head *temp;

	char *buf = kmalloc(sizeof(char) * 100, __GFP_RECLAIM); //100 char should be enough for each floor
	if (buf == NULL) {
		printk(KERN_WARNING "print_wait_floor");
		return -ENOMEM;
	}

	switch(floorNum) {
		case 1:
			list_for_each(temp, &f1.list) { /* forwards*/
				p = list_entry(temp, Passenger, list);
				sprintf(buf, "%s ", p->id);
				strcat(message, buf);
				floor_count++; //adds 1 to floor count
			}
			break;
		case 2:
			list_for_each(temp, &f2.list) { /* forwards*/
				p = list_entry(temp, Passenger, list);
				sprintf(buf, "%s ", p->id);
				strcat(message, buf);
				floor_count++; //adds 1 to floor count
			}
			break;
		case 3:
			list_for_each(temp, &f3.list) { /* forwards*/
				p = list_entry(temp, Passenger, list);
				sprintf(buf, "%s ", p->id);
				strcat(message, buf);
				floor_count++; //adds 1 to floor count
			}
			break;
		case 4:
			list_for_each(temp, &f4.list) { /* forwards*/
				p = list_entry(temp, Passenger, list);
				sprintf(buf, "%s ", p->id);
				strcat(message, buf);
				floor_count++; //adds 1 to floor count
			}
			break;
		case 5:
			list_for_each(temp, &f5.list) { /* forwards*/
				p = list_entry(temp, Passenger, list);
				sprintf(buf, "%s ", p->id);
				strcat(message, buf);
				floor_count++; //adds 1 to floor count
			}
			break;
		case 6:
			list_for_each(temp, &f6.list) { /* forwards*/
				p = list_entry(temp, Passenger, list);
				sprintf(buf, "%s ", p->id);
				strcat(message, buf);
				floor_count++; //adds 1 to floor count
			}
			break;
	}

	strcat(message, "\n");
	kfree(buf);
	return 0;
}


/********************************************************************/


int elevator_proc_open(struct inode *sp_inode, struct file *sp_file) {
	read_p = 1;
	message = kmalloc(sizeof(char) * ENTRY_SIZE, __GFP_RECLAIM | __GFP_IO | __GFP_FS); //allocates 1000 bytes to message
	if (message == NULL) {
		printk(KERN_WARNING "elevator_proc_open");
		return -ENOMEM;
	}

	return 0;
}

//triggers every read (every second with -n1 command)
ssize_t elevator_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset) {

	read_p = !read_p; //idk what this does
	if (read_p)
		return 0;

	//idk how kthreads for movement work but maybe they are called here vvv every read?
		
	//call function that updates state
	//call function that moves elevator to next dest
	//drop off corresponding passengers to that floor
	int addToElevator(current_floor); //while on that floor, add any customers to elevator





	/*****once elevator moves and loads for 1 cycle print all this stuff out*****/

	strcpy(message, "Elevator state: "); //start message to print to procfile

	switch(elevator_state) {
		case OFFLINE:
			strcat(message, "OFFLINE"); //adds to message
			break;
		case IDLE:
			strcat(message, "IDLE");
			break;
		case LOADING:
			strcat(message, "LOADING");
			break;
		case UP:
			strcat(message, "UP");
			break;
		case DOWN:
			strcat(message, "DOWN");
			break;
	}
	strcat(message, "\n");

	strcat(message, "Current floor: ");
	strcat(message, current_floor);
	strcat(message, "\n");

	strcat(message, "Current load: ");
	strcat(message, elevator_weight);
	strcat(message, " lbs");
	strcat(message, "\n");

	strcat(message, "Elevator status: ");
	printElevator();

	if(current_floor == 6)
		strcat(message, "[*] Floor 6: ");
	else 
		strcat(message, "[ ] Floor 6: ");
	printFloorList(6);

	if(current_floor == 5)
		strcat(message, "[*] Floor 5: ");
	else 
		strcat(message, "[ ] Floor 5: ");
	printFloorList(5);

	if(current_floor == 4)
		strcat(message, "[*] Floor 4: ");
	else 
		strcat(message, "[ ] Floor 4: ");
	printFloorList(4);

	if(current_floor == 3)
		strcat(message, "[*] Floor 3: ");
	else 
		strcat(message, "[ ] Floor 3: ");
	printFloorList(3);

	if(current_floor == 2)
		strcat(message, "[*] Floor 2: ");
	else 
		strcat(message, "[ ] Floor 2: ");
	printFloorList(2);

	if(current_floor == 1)
		strcat(message, "[*] Floor 1: ");
	else 
		strcat(message, "[ ] Floor 1: ");
	printFloorList(1);
	
	
	
	

	strcat(message, "Number of passengers: ");
	//sprintf(num_passengers, "%d", elevatorCount(); //prints %d into num_passengers
	strcat(message, elevator.total_cnt); //might be able to just do this and ignore prev line
	
	strcat(message, "Number of passengers waiting: ");
	//sprintf(floor_count, "%d", FloorCountTotal();
	strcat(message, floor_count);

	strcat(message, "Number of passengers serviced: ");
	//sprintf(passengers_serviced, "%d", passengersServiced();
	strcat(message, passengers_serviced);



	int len = strlen(message);	
	copy_to_user(buf, message, len);
	return len;
}

int elevator_proc_release(struct inode *sp_inode, struct file *sp_file) {
	kfree(message);
	return 0;
}

/********************************************************************/

static int elevator_init(void) {
	fops.proc_open = elevator_proc_open;
	fops.proc_read = elevator_proc_read;
	fops.proc_release = elevator_proc_release;

	if (!proc_create(ENTRY_NAME, PERMS, NULL, &fops)) {
		printk(KERN_WARNING "elevator_init\n");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;
	}

	STUB_start_elevator = start_elevator;
    STUB_issue_request = issue_request;
    STUB_stop_elevator = stop_elevator;

	elevator.total_cnt = 0;
	elevator.total_weight = 0;

	INIT_LIST_HEAD(&elevator.list);
	INIT_LIST_HEAD(&f1.list);
	INIT_LIST_HEAD(&f2.list);
	INIT_LIST_HEAD(&f3.list);
	INIT_LIST_HEAD(&f4.list);
	INIT_LIST_HEAD(&f5.list);
	INIT_LIST_HEAD(&f6.list);
	return 0;
}
module_init(elevator_init);

static void elevator_exit(void) {

	STUB_start_elevator = NULL;
    STUB_issue_request = NULL;
    STUB_stop_elevator = NULL;

    remove_proc_entry(ENTRY_NAME, PARENT);
}
module_exit(elevator_exit);
