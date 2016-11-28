//去掉资源中包含的pe文件的mz和pe标志,pData为指向程序文件的首字节
//包含pe文件的资源名为"BIN"
void FixResPeSign(unsigned char * lpData)
{
	//add by cb
	PIMAGE_DOS_HEADER pDosHeader;
	PIMAGE_NT_HEADERS pNtHeader;
	PIMAGE_SECTION_HEADER pSection;//节表
	PIMAGE_RESOURCE_DIRECTORY pResDir;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry;//资源目录Entry
	PIMAGE_RESOURCE_DIR_STRING_U	pResName;//资源名
	PIMAGE_RESOURCE_DATA_ENTRY pResDtEntry;//资源数据Entry
	WORD iSections;//节表数目
	DWORD dResRva;//由DataDirectory获取的资源的RVA
	WORD iResEntryCount;//资源目录Entry数目

	pDosHeader=PIMAGE_DOS_HEADER(lpData);
	pNtHeader=PIMAGE_NT_HEADERS(lpData+pDosHeader->e_lfanew);
	iSections=pNtHeader->FileHeader.NumberOfSections;
	dResRva=pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
	pSection=PIMAGE_SECTION_HEADER(pNtHeader+1);//+1即代表增加改类型的size字节
	//MessageBox(itoa(sizeof(IMAGE_NT_HEADERS),a,10));

	while(iSections--){//遍历节表
		if (pSection->VirtualAddress==dResRva) {//找到了该节,取文件偏移
			pResDir=PIMAGE_RESOURCE_DIRECTORY(lpData+pSection->PointerToRawData);
			iResEntryCount=pResDir->NumberOfIdEntries+pResDir->NumberOfNamedEntries;
			pResDirEntry=PIMAGE_RESOURCE_DIRECTORY_ENTRY(pResDir+1);
			while(iResEntryCount--){
				if (pResDirEntry->NameIsString){
					pResName=PIMAGE_RESOURCE_DIR_STRING_U((char*)pResDir + pResDirEntry->NameOffset);
					if (!wcsncmp(pResName->NameString,L"CDFS",pResName->Length)) {
						//找到了该资源目录
						while(pResDirEntry->DataIsDirectory)
							pResDirEntry=PIMAGE_RESOURCE_DIRECTORY_ENTRY((char*)pResDir+pResDirEntry->OffsetToDirectory+sizeof(IMAGE_RESOURCE_DIRECTORY));
						pResDtEntry=PIMAGE_RESOURCE_DATA_ENTRY(((char*)pResDir)+pResDirEntry->OffsetToData);
						pDosHeader=PIMAGE_DOS_HEADER((char*)pResDir+pResDtEntry->OffsetToData-dResRva);
						pDosHeader->e_magic=0;
						pNtHeader=PIMAGE_NT_HEADERS((char*)pDosHeader+pDosHeader->e_lfanew);
						pNtHeader->Signature=0;
						FixResPeSign((unsigned char *)pDosHeader);//尝试递归修改下层
						//IMAGE_RESOURCE_DATA_ENTRY中的offset为RVA偏移,所以要减去
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