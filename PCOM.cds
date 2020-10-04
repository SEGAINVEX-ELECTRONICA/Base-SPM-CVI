<distribution version="12.0.1" name="PCOM" type="MSI">
	<prebuild>
		<workingdir>workspacedir</workingdir>
		<actions></actions></prebuild>
	<postbuild>
		<workingdir>workspacedir</workingdir>
		<actions></actions></postbuild>
	<msi GUID="{199FD62F-0167-4998-A66A-212C09E21B9C}">
		<general appName="PCOM" outputLocation="d:\Base SPM CVI\cvidistkit.PCOM" relOutputLocation="cvidistkit.PCOM" outputLocationWithVars="d:\Base SPM CVI\cvidistkit.%name" relOutputLocationWithVars="cvidistkit.%name" upgradeBehavior="1" autoIncrement="true" version="1.0.1">
			<arp company="%company" companyURL="" supportURL="" contact="" phone="" comments=""/>
			<summary title="" subject="" keyWords="" comments="" author=""/></general>
		<userinterface language="English" showPaths="true" readMe="" license="">
			<dlgstrings welcomeTitle="PCOM" welcomeText=""/></userinterface>
		<dirs appDirID="101">
			<installDir name="PCOM" dirID="100" parentID="7" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="[Program Files]" dirID="2" parentID="-1" isMSIDir="true" visible="true" unlock="false"/>
			<installDir name="PCOM" dirID="101" parentID="2" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="[Start&gt;&gt;Programs]" dirID="7" parentID="-1" isMSIDir="true" visible="true" unlock="false"/></dirs>
		<files>
			<simpleFile fileID="0" sourcePath="d:\Proyectos\PCOM\cvibuild.PCOM\Release\PCOM.exe" targetDir="101" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="1" sourcePath="d:\Base SPM CVI\copyright.rtf" relSourcePath="copyright.rtf" relSourceBase="0" targetDir="101" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/></files>
		<fileGroups>
			<projectOutput targetType="0" dirID="101" projectID="1">
				<fileID>0</fileID></projectOutput>
			<projectDependencies dirID="101" projectID="1"/></fileGroups>
		<shortcuts>
			<shortcut name="PCOM" targetFileID="0" destDirID="100" cmdLineArgs="" description="" runStyle="NORMAL"/></shortcuts>
		<mergemodules/>
		<products/>
		<runtimeEngine cvirte="true" instrsup="true" lvrt="true" analysis="true" netvarsup="true" dotnetsup="true" activeXsup="true" rtutilsup="true" installToAppDir="false"/><sxsRuntimeEngine>
			<selected>false</selected>
			<doNotAutoSelect>false</doNotAutoSelect></sxsRuntimeEngine>
		<advanced mediaSize="650">
			<launchConditions>
				<condition>MINOS_WINXP_SP0</condition>
			</launchConditions>
			<includeConfigProducts>true</includeConfigProducts>
			<maxImportVisible>silent</maxImportVisible>
			<maxImportMode>merge</maxImportMode>
			<custMsgFlag>false</custMsgFlag>
			<custMsgPath>c:\program files (x86)\national instruments\cvi2012\bin\msgrte.txt</custMsgPath>
			<signExe>false</signExe>
			<certificate></certificate>
			<signTimeURL></signTimeURL>
			<signDescURL></signDescURL></advanced>
		<Projects NumProjects="2">
			<Project000 ProjectID="0" ProjectAbsolutePath="d:\Base SPM CVI\BaseSPM_CVI.prj" ProjectRelativePath="BaseSPM_CVI.prj"/>
			<Project001 ProjectID="1" ProjectAbsolutePath="d:\Proyectos\PCOM\PCOM.prj"/></Projects>
		<buildData progressBarRate="0.715132379942685">
			<progressTimes>
				<Begin>0.000000000000000</Begin>
				<ProductsAdded>0.063741500000469</ProductsAdded>
				<DPConfigured>4.563743500000782</DPConfigured>
				<DPMergeModulesAdded>6.798742499999898</DPMergeModulesAdded>
				<DPClosed>15.148744000000770</DPClosed>
				<DistributionsCopied>18.923753499999435</DistributionsCopied>
				<End>139.834249999999490</End></progressTimes></buildData>
	</msi>
</distribution>
