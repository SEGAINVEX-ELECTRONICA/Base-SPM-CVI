<distribution version="12.0.1" name="BaseSPM_V1" type="MSI">
	<prebuild>
		<workingdir>workspacedir</workingdir>
		<actions></actions></prebuild>
	<postbuild>
		<workingdir>workspacedir</workingdir>
		<actions></actions></postbuild>
	<msi GUID="{BD835D6E-4EBB-4F7F-9C1F-5633EB3A3CB4}">
		<general appName="Base SPM" outputLocation="d:\Base SPM CVI\cvidistkit.Base SPM" relOutputLocation="cvidistkit.Base SPM" outputLocationWithVars="d:\Base SPM CVI\cvidistkit.%name" relOutputLocationWithVars="cvidistkit.%name" upgradeBehavior="1" autoIncrement="false" version="1.0.0">
			<arp company="UAM-SEGAINVEX" companyURL="http://www.uam.es/UAM/Servicio-de-Apoyo-a-la-Investigaci%C3%B3n-(SEGAINVEX)/1242656987472.htm" supportURL="https://github.com/PatricioCoronado/BaseSPM_CVI" contact="patricio.coronado@uam.es" phone="" comments=""/>
			<summary title="" subject="" keyWords="" comments="" author=""/></general>
		<userinterface language="English" showPaths="true" readMe="" license="">
			<dlgstrings welcomeTitle="Base SPM" welcomeText=""/></userinterface>
		<dirs appDirID="101">
			<installDir name="Base SPM" dirID="100" parentID="7" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="Base SPM" dirID="101" parentID="2" isMSIDir="false" visible="true" unlock="false"/>
			<installDir name="[Start&gt;&gt;Programs]" dirID="7" parentID="-1" isMSIDir="true" visible="true" unlock="false"/>
			<installDir name="[Program Files]" dirID="2" parentID="-1" isMSIDir="true" visible="true" unlock="false"/></dirs>
		<files>
			<simpleFile fileID="0" sourcePath="d:\Base SPM CVI\cvibuild.BaseSPM_CVI\Release\BaseSPM_CVI.exe" targetDir="101" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/>
			<simpleFile fileID="1" sourcePath="d:\Base SPM CVI\copyright.rtf" relSourcePath="copyright.rtf" relSourceBase="0" targetDir="101" readonly="false" hidden="false" system="false" regActiveX="false" runAfterInstallStyle="IMMEDIATELY_RESUME_INSTALL" cmdLineArgs="" runAfterInstall="false" uninstCmdLnArgs="" runUninst="false"/></files>
		<fileGroups>
			<projectOutput targetType="0" dirID="101" projectID="0">
				<fileID>0</fileID></projectOutput>
			<projectDependencies dirID="101" projectID="0"/></fileGroups>
		<shortcuts>
			<shortcut name="BaseSPM" targetFileID="0" destDirID="100" cmdLineArgs="" description="" runStyle="NORMAL"/></shortcuts>
		<mergemodules/>
		<products/>
		<runtimeEngine cvirte="true" instrsup="true" lvrt="true" analysis="true" netvarsup="true" dotnetsup="true" activeXsup="true" rtutilsup="true" installToAppDir="false"/><sxsRuntimeEngine>
			<selected>false</selected>
			<doNotAutoSelect>false</doNotAutoSelect></sxsRuntimeEngine>
		<advanced mediaSize="650">
			<launchConditions>
				<condition>MINOS_WIN7</condition>
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
		<Projects NumProjects="1">
			<Project000 ProjectID="0" ProjectAbsolutePath="d:\Base SPM CVI\BaseSPM_CVI.prj" ProjectRelativePath="BaseSPM_CVI.prj"/></Projects>
		<buildData progressBarRate="1.702051614081173">
			<progressTimes>
				<Begin>0.000000000000000</Begin>
				<ProductsAdded>0.106156781250336</ProductsAdded>
				<DPConfigured>7.666414531250401</DPConfigured>
				<DPMergeModulesAdded>8.381177656250131</DPMergeModulesAdded>
				<DPClosed>10.985888187500231</DPClosed>
				<DistributionsCopied>11.427960343750087</DistributionsCopied>
				<End>58.752624875000336</End></progressTimes></buildData>
	</msi>
</distribution>
