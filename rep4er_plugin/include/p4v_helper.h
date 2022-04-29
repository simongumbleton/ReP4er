#pragma once
#include "p4/clientapi.h"


// Sample from p4api
# include "p4/p4libs.h"

int Sample(int argc, char** argv);
int Sample(int argc, char** argv)
{
	ClientUser ui;
	ClientApi client;
	StrBuf msg;
	Error e;

	P4Libraries::Initialize(P4LIBRARIES_INIT_ALL, &e);

	if (e.Test())
	{
		e.Fmt(&msg);
		fprintf(stderr, "%s\n", msg.Text());
		return 1;
	}

	// Any special protocol mods

	// client.SetProtocol( "tag" );

	// Enable client-side Extensions
	// client.EnableExtensions();

	// Connect to server

	client.Init(&e);

	if (e.Test())
	{
		e.Fmt(&msg);
		fprintf(stderr, "%s\n", msg.Text());
		return 1;
	}


	// Run the command "argv[1] argv[2...]"

	client.SetArgv(argc - 2, argv + 2);
	client.Run(argv[1], &ui);

	// Close connection

	client.Final(&e);

	if (e.Test())
	{
		e.Fmt(&msg);
		fprintf(stderr, "%s\n", msg.Text());
		return 1;
	}

	P4Libraries::Shutdown(P4LIBRARIES_INIT_ALL, &e);

	if (e.Test())
	{
		e.Fmt(&msg);
		fprintf(stderr, "%s\n", msg.Text());
		return 1;
	}

	return 0;
}

// end sample from p4api