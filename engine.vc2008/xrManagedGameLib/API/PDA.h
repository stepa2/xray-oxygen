#pragma once
#include "xrGame/PDA.h"

using namespace System;;

namespace XRay
{
	public ref class PDA
	{
	public:
		PDA(IntPtr InNativeObject);

		property bool IsEnabled
		{
			bool get()
			{
				return pNativeObject->IsOn();
			}

			void set(bool value)
			{
				if (value)
				{
					pNativeObject->TurnOn();
				}
				else
				{
					pNativeObject->TurnOff();
				}
			}
		}

		property UInt32 ContactsNum
		{
			UInt32 get()
			{
				return pNativeObject->ActiveContactsNum();
			}
		}

		property float SearchRadius
		{
			float get()
			{
				return pNativeObject->GetRadius();
			}

			void set(float value)
			{
				pNativeObject->SetRadius(value);
			}
		}

		property array<PDA^>^ Contacts
		{
			array<PDA^>^ get()
			{
				PDA_LIST NativePdaList;
				pNativeObject->ActivePDAContacts(NativePdaList);
				array<PDA^>^ Result = gcnew array<PDA ^>((u32)NativePdaList.size());

				for (int i = 0; i < NativePdaList.size(); ++i)
				{
					IntPtr EncapsulatedPtr(NativePdaList[i]);
					Result[i] = gcnew PDA(EncapsulatedPtr);
				}

				return Result;
			}
		}

	private:

		CPda* pNativeObject;
	};
}