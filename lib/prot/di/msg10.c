/*
 * Copyright 2020 Intel Corporation
 * SPDX-License-Identifier: Apache 2.0
 */

/*!
 * \file
 * \brief Implementation of msg number 10; first step of Device Initialize
 * Protocol
 */

#include "util.h"
#include "sdoprot.h"
#include <stdio.h>
#include <stdlib.h>

/* TODO: Move m-string generation here */

/**
 * msg10() - DI.App_start
 * This is the beginning of state machine for ownership transfer of device.The
 * device prepares the "m" string to communicate with the manufacturer, so, it
 * gets the first ownership voucher after Device Initialize (DI) stage is
 * complete.
 *
 * Message format (String defintion is given as part of "m" string generation)
 * ---------------------------------------------------------------------------
 * {
 *    "m": String
 * }
 */
int32_t msg10(sdo_prot_t *ps)
{
	int ret = -1;

	/* Start the "m" string */
	sdow_next_block(&ps->sdow, SDO_DI_APP_START);
	// sdow_begin_object(&ps->sdow);
	// sdo_write_tag(&ps->sdow, "m");
	// sdow_cbor_begin_object(&ps->sdow_cbor);
	sdow_cbor_t *sdow_cbor;
	sdow_cbor = malloc(sizeof(sdow_cbor_t));
	// size_t length;

#if !defined(DEVICE_TPM20_ENABLED)
	/* Get the m-string in the mstring object */
	sdo_byte_array_t *mstring = sdo_byte_array_alloc(sizeof(sdo_byte_array_t));
	ret = ps_get_m_string_cbor(mstring);
	if (ret) {
		LOG(LOG_ERROR, "Failed to get m-string in mstring object\n");
		goto err;
	} else {
		// length = mstring->byte_sz + 100;
		// length = 1024;
		// uint8_t *buffer = (uint8_t*) malloc(length);
		// uint8_t buffer[length];
		/*sdow_cbor->buffer_length = length;
		sdow_cbor->buffer = (uint8_t*) malloc(sdow_cbor->buffer_length);
		CborEncoder random;
		cbor_encoder_init(&sdow_cbor->parent_encoder, sdow_cbor->buffer, sdow_cbor->buffer_length, 0);
		cbor_encoder_create_array(&sdow_cbor->parent_encoder, &sdow_cbor->child_encoder, 1);
		cbor_encoder_create_array(&sdow_cbor->child_encoder, &random, 1);
		cbor_encode_byte_string(&random, mstring->bytes, mstring->byte_sz);
		cbor_encoder_close_container_checked(&sdow_cbor->child_encoder, &random);
		cbor_encoder_close_container_checked(&sdow_cbor->parent_encoder, &sdow_cbor->child_encoder);
		size_t finalLength = cbor_encoder_get_buffer_size(&sdow_cbor->parent_encoder, sdow_cbor->buffer);
		long unsigned i;
		for(i=0; i<finalLength; i++) {
			printf("%02x", sdow_cbor->buffer[i]);
		}
		printf("%s%zd\n", "Raw length", mstring->byte_sz);
		printf("%s%zd\n", "Encoded length", finalLength);*/

		free(sdow_cbor->buffer);
		sdow_cbor->buffer = NULL;
		// sdow_cbor->buffer = (uint8_t*) malloc(sdow_cbor->buffer_length);

		// data: [[[mstring,50],100]]
		sdow_cbor = malloc(sizeof(sdow_cbor_t));
		sdow_init_cbor(sdow_cbor);
		sdow_start_cbor_array(1);
		sdow_start_cbor_array(2);
		sdow_start_cbor_array(2);
		sdow_byte_string(mstring->bytes, mstring->byte_sz);
		sdow_signed_int(50);
		sdow_end_cbor_array();
		sdow_signed_int(100);
		sdow_end_cbor_array();
		sdow_end_cbor_array();

		// [50]
/*		sdow_init_cbor(sdow_cbor);
		sdow_start_cbor_array(1);
		sdow_signed_int(50);
		sdow_end_cbor_array();
*/
		size_t finalLength = cbor_encoder_get_buffer_size(&sdow_cbor->current->cbor_encoder, sdow_cbor->buffer);
		long unsigned i;
		for(i=0; i<finalLength; i++) {
			printf("%02x", sdow_cbor->buffer[i]);
		}
		printf("%s", "Done");
	}

#else
	sdo_byte_array_t *mstring = NULL;
	int mstring_size = get_file_size(DEVICE_MSTRING);

	mstring = sdo_byte_array_alloc(mstring_size + 1);
	if (NULL == mstring) {
		LOG(LOG_ERROR,
		    "Failed to allocate memory for device mstring.\n");
		goto err;
	}

	if (0 != read_buffer_from_file(DEVICE_MSTRING, mstring->bytes,
				       mstring_size)) {
		LOG(LOG_ERROR, "Failed to read %s file!\n", DEVICE_MSTRING);
		sdo_byte_array_free(mstring);
		goto err;
	}

	LOG(LOG_DEBUG, "csr content start: \n%s\ncsr content end\n",
	    mstring->bytes);
	sdo_write_string(&ps->sdow, (char *)mstring->bytes);
	sdo_byte_array_free(mstring);
#endif

	/* End the object */
	// sdow_end_object(&ps->sdow);

	/* This state manages the transition to the next protocol message */
	ps->state = SDO_STATE_DI_SET_CREDENTIALS;
	ret = 0;
err:
	return ret;
}
