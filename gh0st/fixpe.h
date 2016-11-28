//ȥ����Դ�а�����pe�ļ���mz��pe��־,pDataΪָ������ļ������ֽ�
//����pe�ļ�����Դ��Ϊ"BIN"
void FixResPeSign(unsigned char * lpData)
{
	//add by cb
	PIMAGE_DOS_HEADER pDosHeader;
	PIMAGE_NT_HEADERS pNtHeader;
	PIMAGE_SECTION_HEADER pSection;//�ڱ�
	PIMAGE_RESOURCE_DIRECTORY pResDir;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry;//��ԴĿ¼Entry
	PIMAGE_RESOURCE_DIR_STRING_U	pResName;//��Դ��
	PIMAGE_RESOURCE_DATA_ENTRY pResDtEntry;//��Դ����Entry
	WORD iSections;//�ڱ���Ŀ
	DWORD dResRva;//��DataDirectory��ȡ����Դ��RVA
	WORD iResEntryCount;//��ԴĿ¼Entry��Ŀ

	pDosHeader=PIMAGE_DOS_HEADER(lpData);
	pNtHeader=PIMAGE_NT_HEADERS(lpData+pDosHeader->e_lfanew);
	iSections=pNtHeader->FileHeader.NumberOfSections;
	dResRva=pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
	pSection=PIMAGE_SECTION_HEADER(pNtHeader+1);//+1���������Ӹ����͵�size�ֽ�
	//MessageBox(itoa(sizeof(IMAGE_NT_HEADERS),a,10));

	while(iSections--){//�����ڱ�
		if (pSection->VirtualAddress==dResRva) {//�ҵ��˸ý�,ȡ�ļ�ƫ��
			pResDir=PIMAGE_RESOURCE_DIRECTORY(lpData+pSection->PointerToRawData);
			iResEntryCount=pResDir->NumberOfIdEntries+pResDir->NumberOfNamedEntries;
			pResDirEntry=PIMAGE_RESOURCE_DIRECTORY_ENTRY(pResDir+1);
			while(iResEntryCount--){
				if (pResDirEntry->NameIsString){
					pResName=PIMAGE_RESOURCE_DIR_STRING_U((char*)pResDir + pResDirEntry->NameOffset);
					if (!wcsncmp(pResName->NameString,L"CDFS",pResName->Length)) {
						//�ҵ��˸���ԴĿ¼
						while(pResDirEntry->DataIsDirectory)
							pResDirEntry=PIMAGE_RESOURCE_DIRECTORY_ENTRY((char*)pResDir+pResDirEntry->OffsetToDirectory+sizeof(IMAGE_RESOURCE_DIRECTORY));
						pResDtEntry=PIMAGE_RESOURCE_DATA_ENTRY(((char*)pResDir)+pResDirEntry->OffsetToData);
						pDosHeader=PIMAGE_DOS_HEADER((char*)pResDir+pResDtEntry->OffsetToData-dResRva);
						pDosHeader->e_magic=0;
						pNtHeader=PIMAGE_NT_HEADERS((char*)pDosHeader+pDosHeader->e_lfanew);
						pNtHeader->Signature=0;
						FixResPeSign((unsigned char *)pDosHeader);//���Եݹ��޸��²�
						//IMAGE_RESOURCE_DATA_ENTRY�е�offsetΪRVAƫ��,����Ҫ��ȥ
						break;
					}
				}
			}
			//MessageBox(itoa(pSection->PointerToRawData,a,16));
			break;
		}
		pSection++;
	}


}