//(c) Creative Commons non-comercial (share and share a like) copy right
//these lines are to include the OpenCL libaries. apple does things a little different.
//so you will need this inorder to make it compile on Linux or Mac. something I grabbed
//from one tutorial to add the the one from the Mac Lab one. 
//Mac lab: http://developer.apple.com/library/mac/#samplecode/OpenCL_Hello_World_Example/Listings/hello_c.html#//apple_ref/doc/uid/DTS40008187-hello_c-DontLinkElementID_4
//openCL reference: http://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/
//the big blob getting started guide: http://www.thebigblob.com/getting-started-with-opencl-and-gpu-computing/
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#include <stdio.h>

#define DATA_SIZE (1024)

/*This is the kernel. it is what the code that will actually run on the graphics card. 
* Coding for the graphics card is very particular. Firstly, have a few conditionals
* as possible. you'll notice there is one here. I'm not going to go into why here
* because this is about openCL, not graphics card architecture. Just know, ifs are
* really hard for a GPU to deal with.
*
* So, what code can you put on the graphics card? Your best bet is to write it first
* as a for loop. for example:
* for(i = 0; i < count; i++) results[i] = data[i]*data[i];
*
* in openCL it looks like what you have below:
* i = get_global_id(0);
* output[i] = input[i] * input[i];
*
* 'i' represents the same thing. what itteration you are in this pass. for a for loop
* it is a counter. 1,2,3,4,5...count. with a gpu, because things run in parallel, it
* isn't the same. if your gpu had 4 cores say, core 1 calls get_global_id(0) and gets
* the value 0, core 2 calls and gets 1, core 3 calls and gets 2, 4 calls and gets 3.
* they all finish and the next iteration starts. core 1 calls gets 4, and so on. 
* you have no garuntee that your functions will be executed in numerical order.
* core 1 could call and get 42 and core 2 calls and gets 7. that being said, more than
* likely it will execute in sequential order (conditionals will prevent sequential
* order) but never ever depend on it. just decide from the begining that you won't
* bank on that and you won't get any suprises later.
*
* now, you'll notice that this is a string. That is a slick feature of openCL. the
* kernel here is actually sent to the GPU as a string and the GPU compiles it. this
* allows your code to move between graphics cards with out concern. technically it
* will reduce speed for loading. but compiling only happens once, so don't fret.
*
* This code is just plain c code, which is why i advise you to program in c. also
* you'll notice that there is no includes in the kernel. alas, this can't be done. (yet)
* Any data types, functions, structures, etc, that you want you will have to define
* yourself in the kernelsource. at least that is my understanding, how exactly it
* works I don't know yet. I will touch on this issue in a later post.
*/
const char *KernelSource = "\n" \

"__kernel void square(                                                       \n" \

"   __global float* input,                                              \n" \

"   __global float* output,                                             \n" \

"   const unsigned int count)                                           \n" \

"{                                                                      \n" \

"   int i = get_global_id(0);                                           \n" \

"   if(i < count)                                                       \n" \

"       output[i] = input[i] * input[i];                                \n" \

"}                                                                      \n" \

"\n";

 

////////////////////////////////////////////////////////////////////////////////

/* summary:
* Brief summary of what is about to happen:
**	Declare Variables
***	Get Platform ID
***	Get Device ID
***	Create Context
***	Create Command Queue
**	Create program from source
**	Compile/build the program
***	Create the kernal
***	Create the input and output arrays
***	Write the input to the device
***	Set the arguments for the kernel
***	get max group size
***	Execute the kernal
***	Retrieve the results
*	Validate
*	output results
**	Clean up
* 1 '*' means that it this step is not needed, but we are doing it in this example.
** 2 '*' means that it is needed run an openCL program in the way we have here
*** 3 '*' means it is absolutely nessesary to making an openCL program (as far as
* we are concerned at least, there are work around, but that is way beyond my scope)
*/

int main(int argc, char** argv)
{

/*************************************Declare Variables****************************************/

	//most of these are self explanitory. but some, like correct, aren't used till almost the end.
	int err;                            // error code returned from api calls

      	float data[DATA_SIZE];              // original data set given to device

	float results[DATA_SIZE];           // results returned from device

	unsigned int correct;               // number of correct results returned

	size_t global;			    // global domain size for our calculation

	size_t local;			    // local domain size for our calculation

	//something that isn't needed for Macs, again, because they do things slightly different.
	//but it is needed for linux, and doesn't hurt Macs, so platform ID:
	cl_platform_id platform_id;	    // compute platform ID
	cl_device_id device_id;		    // compute device ID
	cl_context context;		    // compute context
	cl_command_queue commands;	    // compute command queue
	cl_program program;		    // compute program
	cl_kernel kernel;		    // compute kernel

	cl_mem input;			    // device memory used for the input array
	cl_mem output;			    // device memory used for the output array

	//Overcoming C:
	/*
	for those of you who are used to java, this isn't to hard of a concept. but
	these variables here are currently pointing to null. for python users, such 
	as myself, this is a little odd, but this is done to have all the declarations
	out of the way before we get into the meat of the code. Personally I don't
	really agree with it, but it allows you to see in a shot what we are about
	to do, so I left it in.
	*/

	//fill our data set with values.
	//this is just dumby code to make it look like we are working on something.
	int i = 0;
	unsigned int count = DATA_SIZE;
	for(i = 0; i < count; i++)
		data[i] = i;

/************************************Get Platform ID****************************************/
	//get platform IDs:
	//I'm not entirely sure what the difference between platform ID and device ID.
	//from what I can gather, a platform ID can signifiy the difference between
	//a gpu and a cpu as well as seperate implimentations of openCL. (if you, say
	//has more than one version of openCL running on the same device)

	printf("getting platform ID... ");

	int platsAvaliable;

	//this function requests 1 platform ID.
	err = clGetPlatformIDs(1,&platform_id,&platsAvaliable);

	/*first argument is the number of platforms you want returned. we only care for 1
	* which by default will be your graphics card.
	*
	* second argument is a pointer to our variable platform_id. because we passed a 
	* pointer, the magic of c will change the value in platform to be our platform(s)
	* that is(are) avaliable.
	*
	* the last argument will return the number of platforms you retrieved if it isn't
	* null. in this case I believe it should, and it will only give you one. now, that
	* sounds pointless, considering you specify how many you want above. but where this
	* is important is if you were to ask for 5 platforms and only 2 were found, this
	* is how you would know. again, through pointer magic, we get a value assigned to it.
	*/

	if (err != CL_SUCCESS)
	{
		printf("\nError: Failed to recieve platformID\n");
		return 1;
	}

	printf("Done\n");


/************************************Get Device ID******************************************/

	//This function will retrieve the device ID that openCL has assigned to it so that we
	//can tell openCL what device we want to talk to in later steps.

	printf("Getting compute device... ");	
	// connect to compute device
	err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
	/*First argument: Platform ID. this we got from the last step, it is the platform 
	* under which we intend on operating.
	* Second argument: device type. the will distinguish between GPUs and CPUs, what 
	* are called accelerators (IBM cell broad band processor). 
	* Third argument: num_entries. this is the number of devices you want to retrieve.
	* Fourth argument: devices. this is a our variable device_id. right now it points
	* to null, but after this step, through pointer magic, it will point to our device.
	* Fifth argument: num_devices. this is the number of devices available that match
	* device_type. most likely this will be 1 so far as we are concerned, get into 
	* super computers and this might actually matter to you. set it to null and
	* openCL just ignores it.
	*/
	if (err != CL_SUCCESS)
	{
		printf("\nError: Failed to create a device group.\n");
		return 2;
	}
	printf("Done\n");

/************************************Create Context*****************************************/

	//This function creates a openCL context. a context is used by open cl to manage the
	//objects that are related to it. things such as command queues, memory, programs,
	//kernels, and for executing kernels on one or more devices that will be specified
	//in the context, which we will do here:
	printf("creating compute context... ");
	//create compute context

	context = clCreateContext(0,1, &device_id, NULL, NULL, &err);
	/*First argument: properties. here you can specify which platform to use, if you have 
	* more than 1 platform in your device_id (which can be a list. This is also where you
	* can in able cl and gl sharing. I'll touch on this later, but just an fyi
	* Second argument: num_devices. the number of devices specified in the devices argument.
	* Third argument: devices. point to a list of unique devices returned from the last 
	* step. duplicate devices are ignored.
	* Fourth argument: pfn_notify. this is a callback function that can be registered by the
	* application. it is ued for reporting information on errors that occur in this context.
	* if you set it to null openCL will ignore, and thats where we'll leave it for now.
	* Fifth argument: user_data. this only matters if you have something for pfn_notify. user-data is
	* the argument passed to pfn_notify when it is called. null will work for now.
	* sixth argument: error code reporting. this is a way to get the error code with out
	* it having to be in the return value. a simple trick that you'll se frequently in opencl.
	*/

	if (!context)
	{
		printf("\nError: Failed to create a compute context.\n");
		return 3;
	}
	printf("Done\n");

/************************************Create command Queue***********************************/

	//This function creates the command queue. this holds the commands, so no commands
	//are actually made here. 
	printf("creating commands... ");
	commands = clCreateCommandQueue(context,device_id, 0, &err);
	/*First Argument: context. the context
	* Second Argument: device. the device.
	* Third argument: properties. this is the properties for this queue. there are 2
	* options. CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE. this does as it sounds. if this
	* flag is enabled, then the commands in this queue are allowed to be executed out
	* of order of the recieveing. note that here command does not refer to each itteration
	* like in my for loop comparison at the top, rather it refers to the entirity of
	* that loop. so not individual operations or runs, but the whole bundle. the other
	* option is CL_QUEUE_PROFILING_ENABLE. this is a little out of my scope. this is used
	* to profile commands as the leave the queue. what exactly you can find out is not
	* clear to me, but I imagine errors and execution time.
	* Fourth argument: error. i told you you'll see this frequently. next one too.
	*/

	if (!commands)
	{
		printf("\nError: Failed to create a command queue. \n");
		return 4;
	}
	printf("Done\n");

/************************************Create program from source*****************************/

	//this command takes a string and pulls it into the openCL interface. depending on
	//your device, and your implementation, it may or may not be stored on the device
	//at this point. but it is stored as a string where ever it is.
	printf("Creating compute program from source buffer... ");
	program = clCreateProgramWithSource(context, 1, (const char **) & KernelSource, NULL, &err);
	/*First Argument: context. the compute context.
	* Second Argument: count. this is the number of pointers in the c string following:
	* Third Argument: strings. an array of pointers to arrays of characters, also known
	* as a c string. This is your source, stored as a string.
	* Fourth Argument: Lengths. this is the length of the strings from 3rd argument. if
	* it is null, as it is here, it assumes that the strings are null terminated. which
	* they should be in all honesty. if they aren't, you have to include the length of 
	* the strings here. if you have more than one string, you will have to include the
	* length(unless they are all null terminated) and any that are null terminated you 
	* can list as 0. though, theoretically, giving the length will save your device some
	* small time. but Null will suffice for now and in almost all cercumstances.
	* Fifth argument: errors!
	*/



	if (!program)
	{
		printf("\nError: failed to create compute program.\n");
		return 5;
	}
	printf("Done\n");

/************************************Compile/build the program******************************/
	//this function builds(compiles and links) the source into a binary. this is nessesary
	//to actualy run the code, and in the process, makes it specific to your device.
	printf("Building the executable... ");
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	/*First Argument: the program object from above.
	* Second Argument: num_devices. this is the number of devices in device list:
	* Third Argument: device_list. this is the list of devices associated with the program
	* which you want it to be compiled for. if it is null it just compiles the program
	* for all devices associated with the program (associated through the context)
	* Fourth argument: Options. a pointer to a null-terminated string of characters that 
	* describes the build options used for building the program executable. these options
	* can be found here: http://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/ under
	* clBuildProgram. They are beyond my scope for now. if you are really interested, go there.
	* Fifth argument: pfn_notify function pointer for notification routine. just like
	* with create context. 
	* Sixth argument: user_data. just like wiht create context. this is sent to the pfn_notify
	* function as an argument when it is called. 
	*/
	if (err != CL_SUCCESS)
	{
		printf("Error: Failed to build program executable. \n");
		return 6;
	}
	printf("Done\n");
	/*I mentioned above that these last two steps are not absolutely nessesary. The reason
	* is that these create a binary from a source. this is a fast(in code time) and simple
	* method of doing this, but there is a better way in terms of performance. particularly
	* if you have a fairly large program. compiling every time you run can prove to be 
	* annoying/time consuming. Using a different api call called "clCreateProgramWithBinary"
	* one can skip these last 2 steps. If understand the openCL reference pages. which
	* is not nessassirly an accurate assumption... (at least on this case) */
/************************************Create the kernel**************************************/
	//This step finally creates the entirity of the kernel, on the device and ready to use.
	printf("Creating the kernal... ");
	kernel = clCreateKernel(program, "square", &err);
	/*First argument: program. from the steps above.
	* Second argument: kernel_name. the kernel's name. can be used to reference it later.
	* Third argument: Error codes! w00h!
	*/

	if (!kernel || err != CL_SUCCESS)
	{
		printf("Error: failed to create compute kernel.\n");
		return 7;
	}
	printf("Done\n");

/*********************************create input and output arrays****************************/
	//this function allocates memory on the device:
	printf("Creating the input and output arrays... ");
	input = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * count, NULL, NULL);
	if (input) 
		printf("Input Done... ");
	output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * count, NULL, NULL);
	/*First Argument: context. the context.
	* Second Argument: Flags. these are the flags taht will be applied to the memory that
	* is going to be alocated. This can be used for fancy sharing of memory, but right now
	* the only flags that we will be concerned with are, read only, write only, read & write
	* which are formated as such:
	* CL_MEM_READ_WRITE
	* CL_MEM_WRITE_ONLY
	* CL_MEM_READ_ONLY
	* the others deal with host pointers, which is beyond my scope for now. 
	* Third Argument: The size of the buffer to be allocated. this is always the best
	* way to get that size. sizeof(type) * number of type. never hard code this.
	* Fourth Argument: host_ptr. this only matters if you are using one of those fancy 
	* memory flags I said I'm not going to be using for now.
	* Fifth Argument: error code fun. I don't know why it is null here.
	*/
	if (!input || !output)
	{
		printf("Error: Failed to allocate device memory.\n");
		return 8;
	}
	printf("Output Done \n");

/**********************************Write the input to the device****************************/
	//this writes data to the previously allocated memory on the device.
	printf("write data set to input array... ");
	err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0, sizeof(float) * count, data, 0, NULL, NULL);
	/*oh boy....
	1st Arg: CL command queue. the command queue made above.
	2nd Arg: buffer. the memory buffer that you want to write to.
	3rd Arg: blocking_write. Indicites if write operations are blocking or non-blocking
		If you know anything about how threads, this should be fairly obvious. but
		For those who don't, and because I want to be clear, blocking means that the
		program will not move past this step until the operation has been completed,
		non-blocking means that it will, and the write process will continue to run
		in the background. with blocking the pointer to the local copy of the address
		is the memory pointed to by the 6th arg will be free to be reused/deleted.
		with non blocking you will have to wait for the event in the 9th arg.
	4th Arg: offset. the offfset in bytes in the buffer object to write to. what that
		means is basically where to start. if you allocate 100 bytes and your offset
		is 20, the first byte will be written to the 21st spot (buffer[20] in array
		notation as buffers are 0 indexed). this can allow you to save info at the
		begining, or make more then one write. write 10 bytes, then 20 (offset by 10),
		then 30 (offset by 30), then 40(offset by 60). so, remember then also, that
		if you make more then one write, you don't clobber your previous data.
	5th Arg: cb(copy bytes I think?). number of bytes of data being written.
	6th Arg: ptr. the pointer to the data location in host memory. in this case, data.
	7th Arg: Event_wait_list, 8th Arg: nm_events_in_wait_list. specifies the events and
		how many needed to complete before this particular command can be executed.
		you can call for this kernel to be executed before it is ready by using these
		events. the kernel will not execute until all the events in this list have
		been set. these events notify the device and are set by the host(usually).
	9th Arg: event. this event is used to notifiy the host. it notifies the host that
		all the bytes from ptr(in our case, 'data') have been copied over and ptr 
		is now free for the host to do with as it please with out concern for causing
		problems to the device.
	*/

	if (err != CL_SUCCESS)
	{
		printf("Error: Failed to write to source array.\n");
		return 9;
	}
	printf("Done\n");

/**********************************Set the arguments for the kernel*************************/
	//these functions are used to set what the arguments are for the kernel.
	printf("setting the arguments to the kernel... ");
	err = 0;
	
	err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
	err |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &count);
	/*For your reference, the kernel's prototype was: 
	* __kernel void square(__global float* input, __global float* output, const unsigned int count)
	* so this function wants 2 arrays of float, and an unsigned int.
	* 1st Arg: kernel. the kernel object.
	* 2nd Arg: arg_index. which argument you are going to be setting. in this case:
	*	0 == input
	*	1 == output
	*	2 == count
	* 3rd Arg: arg_size. how much memory the argument. you can have locations in host memory
	* 	be ponited to here, though greatly un-advised for anything other than like
	* 	an int as we can see here. and preferably nothing at all (data transfer is
	*	a HUGE bottle neck. the less you do it, the better. in the case of the first 
	*	two it is the size of a pointer to a cl_mem object. which is probably 64-bits
	*	or 32-bits depending on your computer.
	* 4th Arg: arg_value. what the argument actually is. in this case you can see: input,
	* 	output, and count. all assigned to their appropriate index.
	*/

	if(err != CL_SUCCESS)
	{
		printf("Error: failed to set kernel arguments. %d\n", err);
		return 10;
	}
	printf("Done\n");

/***********************************get max group size**************************************/
	//this retrieves the maximum size of a work group. this information can be used to
	//optimize your kernel code. I have it print here because your probably curious, i was.
	printf("Get the max work group size... ");
	err = clGetKernelWorkGroupInfo(kernel,device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
	/*1st Arg: kernel. the kernel which you are concerned with. 
	* 2nd Arg: device. the device with which you are concerned.
	* 3rd Arg: param_name. you want to get from info. the paramaters can be found again
	* 	On Khronos's reference pages for openCL under clGetKernelWorkGroupInfo:
	*	http://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/
	* 4th Arg: param_value_size. this is the size, in bytes, of memory being pointed to by 
	*	the 5th arg.
	* 5th Arg: param_vaue. pointer to memory where the appropriate result being queried is 
	*	returned to. if it is null, this is ignored, which makes this whole call seem
	*	really pointeless (no pun intended) so don't forget to put a pointer there.
	* 6th Arg: returns the actual size in bytes of data copied to param_value. if it is
	*	set to NULL, then this is ignored.
	*/

	if (err != CL_SUCCESS)
	{
		printf("Error: Failed to retrieve Kernal work group info.\n");
		return 11;
	}
	printf("Done %zu\n",local);
	//how did i know that was %zu? here:
	//http://stackoverflow.com/questions/1546789/clean-code-to-printf-size-t-in-c-or-nearest-equivalent-of-c99s-z-in-c


/************************************execute the kernal*************************************/
	// Execute the kernel over the entire range of our 1d input data set
	// using the maximum number of work group items for this device
	printf("Executing the kernal... ");
	global = count;
	err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
	/*1st Arg: command_queue. the target command queue.
	* 2nd Arg: kernel. the kernel that is to be run.
	* 3rd Arg: work_dim. the number of dimensions used to specify the gobal work-items and
	*	work-items in the work-group. work_dim must be greater than zero and less than
	*	or equal to CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS.
	*	not entirely sure what that means. I'm guessing that it is asking if you nested
	*	arrays. for example a 2d array, which would make this argument 2, but because
	*	it is a 1d array we are using, the argument here is 1.
	* 4th Arg: global_work_offset. this matters mostly if you have a multi-demenional array.
	*	this specifies at which array you wish to start. not exactly sure how it works.
	* 5th Arg: global_work_size. this is how far into the dimension list above you wish to go
	* 6th Arg: local_work_size. points to an array of work_dim unsigned values that defines
	* 	how many work-items make up a work group. we are sending to it the max work 
	* 	group size, and this is probably what you'll always do.
	* 7th Arg: num_events_in_wait_list, 8th Arg: Event_wait_list. specify events that need
	*	to complete before this command starts. just like enqueue write above.
	*	I neglected to mention above that if this is null, then the kernel doesn't wait.
	* 9th Arg: event. this is an event to notify the host that this kernel is done executing.
	*	if it is null, then no event is made or returned.
	*/
	

	if (err)
	{
		printf("\nError: Failed to execute kernel!\n");
		return 12;
	}
	printf("Done\n");
	
	printf("Waiting for kernel to finish... ");
	clFinish(commands);//alternative to getting an event, you can just tell your program
		//to block(wait) until the command queue you pass here finishes. this doesn't
		//provide the granularity of control that the event above does, but is simple
		//and works for our needs.
	printf("Done\n");


/*************************************Retrieve the results**********************************/
	//This function retrieves the information from the device so that you can see it.
	printf("Retrieveing the results... ");
	err = clEnqueueReadBuffer( commands, output, CL_TRUE, 0, sizeof(float) * count, results, 0, NULL, NULL );
	/*1st Arg: command_queue. the command to which you are concerned.
	* 2nd Arg: buffer. the buffer you want it to read from.
	* 3rd Arg: is this read blocking? refer to the enqueue write if you have forgotten 
	*	what this means
	* 4th Arg: offset. the offset in the output buffer where you want it to start.
	* 5th Arg: cb. how many bytes to transfer.
	* 6th Arg: ptr. where you want the info you are retrieveing to be put.
	* 7th Arg: num_events_in_wait_list, 8th Arg: Event_wait_list. specify events that need
		complete before this command starts. you've seen this before.
	* 9th Arg: event. the event that notifies you that this command has been completed.
	*/
	if (err != CL_SUCCESS)
	{
		printf("\nError: Could not read output array\n");
		return 13;
	}
	printf("Done\n");

/*************************************Validate**********************************************/
	//this is done to validate your results. it is good practice that offline you always
	//run your code out side of openCL to make sure you are getting the right results by
	//using a known working algorithm. This paradigm will probably change in the future,
	//but I'm still in college myself, so what do I know. (soap box: that this paradigm
	// is a little archaic and backwards...)
	printf("Validating results... ");

	for(i=0; i < count; i++)
	{//but this works well for this example to show you it is working.
		if(results[i] == data[i] * data[i])
			correct++;
	}

	printf("Done\n");

/**************************************output results***************************************/
	//out put results for happiness.
	printf("Computed '%d/%d' correct values!\n", correct, count);

/**************************************clean up*********************************************/
	//these you should always always do at the end of your program.
	printf("Begining cleanup... ");
	clReleaseMemObject(input);

	clReleaseMemObject(output);

	clReleaseProgram(program);

	clReleaseKernel(kernel);

	clReleaseCommandQueue(commands);

	clReleaseContext(context);

	printf("Done\n");
	//forget these in a program that makes multiple contexts, kernels, command queues,
	//etc. and you'll start facing memory leaks. you technically can get away with not
	//having them in a program of this size, because after the program ends your OS will
	//free up the memory. but that is a bad practice to get into. so remember to 
	//release your memory. it doesn't take a lot and can save you a lot of problems.
/*******************************************************************************************/
	//joyful explitive.
	printf("BOOM BABY!\n");
	return 0;
}

