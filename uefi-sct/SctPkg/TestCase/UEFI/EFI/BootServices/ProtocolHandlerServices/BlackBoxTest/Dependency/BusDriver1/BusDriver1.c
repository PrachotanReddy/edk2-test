/** @file

  Copyright 2006 - 2017 Unified EFI, Inc.<BR>
  Copyright (c) 2010 - 2017, Intel Corporation. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at 
  http://opensource.org/licenses/bsd-license.php
 
  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
 
**/
/*++

Module Name:

  BusDriver1.c

Abstract:

  for Protocol Handler Boot Services Black Box Test

--*/

#include "ProtocolDefinition.h"
#include "Misc.h"
#include "../Inc/TestDriver.h"

//
// TestBusDriver1:
// Consume InterfaceTestProtocol1;
// Can _CReate 10 child handles and install InterfaceTestProtocol2 onto each
// child handle.
//

//
// data definition here
//
typedef struct {
  EFI_DRIVER_BINDING_PROTOCOL           DriverBinding;
  INTERFACE_FUNCTION_TEST_PROTOCOL_2    InterfaceFunctionTestProtocol2;
} BUS_DRIVER_1_PRIVATE_DATA;

#define BUS_DRIVER_1_PRIVATE_DATA_FROM_DRIVER_BINDING(a) \
  BASE_CR(a, BUS_DRIVER_1_PRIVATE_DATA, DriverBinding)

BUS_DRIVER_1_PRIVATE_DATA          *mPrivateData;


EFI_STATUS
EFIAPI
InitializeBusDriver1 (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
BusDriver1BindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
BusDriver1BindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
BusDriver1BindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );


void
InitializeDriverBinding (
  EFI_DRIVER_BINDING_PROTOCOL *DriverBinding
  );

VOID
EFIAPI
Protocol2ReportProtocolAttributes(
  IN INTERFACE_FUNCTION_TEST_PROTOCOL_2   *This,
  OUT PROTOCOL_ATTRIBUTES   *ProtocolAttributes
  );

EFI_STATUS
EFIAPI
BusDriver1Unload (
  IN EFI_HANDLE       ImageHandle
  );



//
// global variable for this test driver's image handle
//


EFI_STATUS
EFIAPI
InitializeBusDriver1 (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                            Status;
  EFI_LOADED_IMAGE_PROTOCOL             *LoadedImageInfoPtr;

  EfiInitializeTestLib (ImageHandle, SystemTable);

  //
  // allocate memory for PrivateData
  //
  Status = gtBS->AllocatePool (
                        EfiBootServicesData,
                        sizeof (BUS_DRIVER_1_PRIVATE_DATA),
                        (VOID**)&mPrivateData
                        );
  if (EFI_ERROR(Status)) {
    return Status;
  }
  gtBS->SetMem (mPrivateData,sizeof (BUS_DRIVER_1_PRIVATE_DATA),0);

  InitializeDriverBinding (&mPrivateData->DriverBinding);

  Status = gtBS->InstallProtocolInterface (
            &ImageHandle,
            &gEfiDriverBindingProtocolGuid,
            EFI_NATIVE_INTERFACE,
            &mPrivateData->DriverBinding
            );
  mPrivateData->DriverBinding.ImageHandle = ImageHandle;
  mPrivateData->DriverBinding.DriverBindingHandle = ImageHandle;
  //
  // UnLoad Function Handler
  //
  gtBS->HandleProtocol (
        ImageHandle,
        &gEfiLoadedImageProtocolGuid,
        (VOID*)&LoadedImageInfoPtr
        );

  LoadedImageInfoPtr->Unload = BusDriver1Unload;

  return EFI_SUCCESS;

}

EFI_STATUS
EFIAPI
BusDriver1BindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS        Status;
  EFI_DEV_PATH      *Node;

  if (RemainingDevicePath != NULL) {
    Node = (EFI_DEV_PATH *)RemainingDevicePath;
    if (Node->DevPath.Type != HARDWARE_DEVICE_PATH ||
        Node->DevPath.SubType != HW_VENDOR_DP ||
        SctDevicePathNodeLength(&Node->DevPath) != sizeof(VENDOR_DEVICE_PATH)) {
      return EFI_UNSUPPORTED;
    }
  }

  Status = gtBS->OpenProtocol (
                      Controller,
                      &mInterfaceFunctionTestProtocol1Guid,
                      (VOID **) NULL,
                      This->DriverBindingHandle,
                      Controller,
                      EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                      );
  if (EFI_ERROR(Status)) {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
BusDriver1BindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS                            Status,BadStatus;
  INTERFACE_FUNCTION_TEST_PROTOCOL_1    *IFTestProt1;
  BUS_DRIVER_1_PRIVATE_DATA             *PrivateData;
  EFI_DEVICE_PATH_PROTOCOL              *DevicePathArray[10];
  EFI_GUID                              VendorGuidArray[10];
  EFI_DEV_PATH                          *Node;
  EFI_HANDLE                            Handle;
  UINTN                                 Index, EndIndex;

  gtBS->SetMem (DevicePathArray,sizeof (EFI_DEVICE_PATH_PROTOCOL*) * 10, 0);

  VendorGuidArray[0] = mVendorDevicePath1Guid;
  VendorGuidArray[1] = mVendorDevicePath2Guid;
  VendorGuidArray[2] = mVendorDevicePath3Guid;
  VendorGuidArray[3] = mVendorDevicePath4Guid;
  VendorGuidArray[4] = mVendorDevicePath5Guid;
  VendorGuidArray[5] = mVendorDevicePath6Guid;
  VendorGuidArray[6] = mVendorDevicePath7Guid;
  VendorGuidArray[7] = mVendorDevicePath8Guid;
  VendorGuidArray[8] = mVendorDevicePath9Guid;
  VendorGuidArray[9] = mVendorDevicePath10Guid;

  PrivateData = BUS_DRIVER_1_PRIVATE_DATA_FROM_DRIVER_BINDING (This);

  Status = gtBS->OpenProtocol (
                     Controller,
                     &mInterfaceFunctionTestProtocol1Guid,
                     (VOID **) &IFTestProt1,
                     This->DriverBindingHandle,
                     Controller,
                     EFI_OPEN_PROTOCOL_BY_DRIVER
                     );
  if (EFI_ERROR(Status) && (Status != EFI_ALREADY_STARTED)) {
    return Status;
  }


  if (RemainingDevicePath != NULL) {
    Node = (EFI_DEV_PATH*)RemainingDevicePath;
    CreateVendorDevicePath (&DevicePathArray[0],Node->Vendor.Guid);
  } else {
    for (Index = 0; Index < 10; Index ++) {
      CreateVendorDevicePath (&DevicePathArray[Index],VendorGuidArray[Index]);
    }
  }

  if (RemainingDevicePath == NULL) {
    EndIndex = 10;
  } else {
    Node = (EFI_DEV_PATH*)RemainingDevicePath;
    EndIndex = 1;
  }

  InitializeInterfaceFunctionTestProtocol2 (&PrivateData->InterfaceFunctionTestProtocol2);

  BadStatus = EFI_SUCCESS;
  for (Index = 0; Index < EndIndex; Index ++) {

    Handle = NULL;

    Status = gtBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &mInterfaceFunctionTestProtocol2Guid,
                  &PrivateData->InterfaceFunctionTestProtocol2,
                  &gEfiDevicePathProtocolGuid,
                  DevicePathArray[Index],
                  NULL
                  );
    if (EFI_ERROR(Status)) {
      BadStatus = Status;
      FreeVendorDevicePath (DevicePathArray[Index]);
      DevicePathArray[Index] = NULL;
      continue;
    }

    Status = gtBS->OpenProtocol (
                     Controller,
                     &mInterfaceFunctionTestProtocol1Guid,
                     (VOID **) &IFTestProt1,
                     This->DriverBindingHandle,
                     Handle,
                     EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                     );
    if (EFI_ERROR(Status)) {
      BadStatus = Status;
      //
      // restore environment
      //
      gtBS->UninstallMultipleProtocolInterfaces (
                  Handle,
                  &mInterfaceFunctionTestProtocol2Guid,
                  &PrivateData->InterfaceFunctionTestProtocol2,
                  &gEfiDevicePathProtocolGuid,
                  DevicePathArray[Index],
                  NULL
                  );
      FreeVendorDevicePath (DevicePathArray[Index]);
      DevicePathArray[Index] = NULL;
    }
  }

  return BadStatus;
}

EFI_STATUS
EFIAPI
BusDriver1BindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )
{
  BUS_DRIVER_1_PRIVATE_DATA          *PrivateData;
  EFI_STATUS                         Status;
  BOOLEAN                            AllChildrenStopped;
  EFI_DEVICE_PATH_PROTOCOL           *DevicePath;
  UINTN                              Index;

  PrivateData = BUS_DRIVER_1_PRIVATE_DATA_FROM_DRIVER_BINDING (This);

  if (NumberOfChildren == 0) {

    gtBS->CloseProtocol (
            Controller,
            &mInterfaceFunctionTestProtocol1Guid,
            This->DriverBindingHandle,
            Controller
            );
    return EFI_SUCCESS;
  }

  AllChildrenStopped = TRUE;

  for (Index = 0; Index < NumberOfChildren; Index++) {

    Status = gtBS->OpenProtocol (
                  ChildHandleBuffer[Index],
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &DevicePath,
                  This->DriverBindingHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
    if (EFI_ERROR (Status)) {
      AllChildrenStopped = FALSE;
      continue;
    }

    Status = gtBS->CloseProtocol (
            Controller,
            &mInterfaceFunctionTestProtocol1Guid,
            This->DriverBindingHandle,
            ChildHandleBuffer[Index]
            );
    if (!EFI_ERROR(Status)) {

      Status = gtBS->UninstallMultipleProtocolInterfaces (
                    ChildHandleBuffer[Index],
                    &mInterfaceFunctionTestProtocol2Guid,
                    &PrivateData->InterfaceFunctionTestProtocol2,
                    &gEfiDevicePathProtocolGuid,
                    DevicePath,
                    NULL
                    );
    }

    if (EFI_ERROR (Status)) {
      AllChildrenStopped = FALSE;
    } else {
      FreeVendorDevicePath (DevicePath);
    }
  }

  if (!AllChildrenStopped) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

void
InitializeDriverBinding (
  EFI_DRIVER_BINDING_PROTOCOL *DriverBinding
  )
{
  DriverBinding->Supported            = BusDriver1BindingSupported;
  DriverBinding->Start                = BusDriver1BindingStart;
  DriverBinding->Stop                 = BusDriver1BindingStop;
  DriverBinding->Version              = 0x10;
  DriverBinding->ImageHandle          = NULL;
  DriverBinding->DriverBindingHandle  = NULL;
}

/**
 *  The driver's Unload function
 *  @param  ImageHandle The test driver image handle
 *  @return EFI_SUCCESS Indicates the interface was Uninstalled
*/
EFI_STATUS
EFIAPI
BusDriver1Unload (
  IN EFI_HANDLE       ImageHandle
  )
{
  //
  // free resources
  //
  gtBS->UninstallProtocolInterface (
              mPrivateData->DriverBinding.DriverBindingHandle,
              &gEfiDriverBindingProtocolGuid,
              &mPrivateData->DriverBinding
              );
  gtBS->FreePool (mPrivateData);

  return EFI_SUCCESS;
}
