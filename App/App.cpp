#include <cstdio>
#include <cstring>
#include <iostream>
#include "Enclave_u.h"
#include <sgx_urts.h>
#include "error_print.h"



sgx_enclave_id_t global_eid = 0;



/* OCALL implementations */
void ocall_print(const char* str)
{
	std::cout << "Output from OCALL: " << std::endl;
	std::cout << str << std::endl;
	
	return;
}



/* Enclave initialization function */
int initialize_enclave()
{
	std::string launch_token_path = "enclave.token";
	std::string enclave_name = "enclave.signed.so";
	const char* token_path = launch_token_path.c_str();

	sgx_launch_token_t token = {0};
	sgx_status_t status = SGX_ERROR_UNEXPECTED;
	int updated = 0;


	/*==============================================================*
	 * Step 1: Obtain enclave launch token                          *
	 *==============================================================*/
	
	/* If exist, load the enclave launch token */
	FILE *fp = fopen(token_path, "rb");

	/* If token doesn't exist, create the token */
	if(fp == NULL && (fp = fopen(token_path, "wb")) == NULL)
	{		
		/* Storing token is not necessary, so file I/O errors here
		 * is not fatal
		 */
		std::cerr << "Warning: Failed to create/open the launch token file ";
		std::cerr << "\"" << launch_token_path << "\"." << std::endl;
	}


	if(fp != NULL)
	{
		/* read the token from saved file */
		size_t read_num = fread(token, 1, sizeof(sgx_launch_token_t), fp);

		/* if token is invalid, clear the buffer */
		if(read_num != 0 && read_num != sizeof(sgx_launch_token_t))
		{
			memset(&token, 0x0, sizeof(sgx_launch_token_t));

			/* As aforementioned, if token doesn't exist or is corrupted,
			 * zero-flushed new token will be used for launch.
			 * So token error is not fatal.
			 */
			std::cerr << "Warning: Invalid launch token read from ";
			std::cerr << "\"" << launch_token_path << "\"." << std::endl;
		}
	}


	/*==============================================================*
	 * Step 2: Initialize enclave by calling sgx_create_enclave     *
	 *==============================================================*/

	status = sgx_create_enclave(enclave_name.c_str(), SGX_DEBUG_FLAG, &token,
		&updated, &global_eid, NULL);
	
	if(status != SGX_SUCCESS)
	{
		/* Defined at error_print.cpp */
		sgx_error_print(status);
		
		if(fp != NULL)
		{
			fclose(fp);
		}

		return -1;
	}

	/*==============================================================*
	 * Step 3: Save the launch token if it is updated               *
	 *==============================================================*/
	
	/* If there is no update with token, skip save */
	if(updated == 0 || fp == NULL)
	{
		if(fp != NULL)
		{
			fclose(fp);
		}

		return 0;
	}


	/* reopen with write mode and save token */
	fp = freopen(token_path, "wb", fp);
	if(fp == NULL) return 0;

	size_t write_num = fwrite(token, 1, sizeof(sgx_launch_token_t), fp);

	if(write_num != sizeof(sgx_launch_token_t))
	{
		std::cerr << "Warning: Failed to save launch token to ";
		std::cerr << "\"" << launch_token_path << "\"." << std::endl;
	}

	fclose(fp);

	return 0;
}




int main()
{
	/* initialize enclave */
	if(initialize_enclave() < 0)
	{
		std::cerr << "App: fatal error: Failed to initialize enclave.";
		std::cerr << std::endl;
		return -1;
	}


	/* start ECALL */
	const char *message = "Hello Enclave.";
	size_t message_len = strlen(message);
	int retval = -9999;

	std::cout << "Execute ECALL.\n" << std::endl;

	sgx_status_t status = ecall_test(global_eid, &retval,
		message, message_len);

	if(status != SGX_SUCCESS)
	{
		sgx_error_print(status);

		return -1;
	}
	else
	{
		/* This function also can display succeeded message */
		sgx_error_print(status);
	}


	/* print ECALL result */
	std::cout << "\nReturned integer from ECALL is: " << retval << std::endl;
	std::cout << std::endl;



	/* Destruct the enclave */
	sgx_destroy_enclave(global_eid);

	std::cout << "Whole operations have been executed correctly." << std::endl;

	return 0;
}
