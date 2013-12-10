<?xml version='1.0' encoding='UTF-8'?>
<Project Type="Project" LVVersion="12008004">
	<Item Name="My Computer" Type="My Computer">
		<Property Name="server.app.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.control.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.tcp.enabled" Type="Bool">false</Property>
		<Property Name="server.tcp.port" Type="Int">0</Property>
		<Property Name="server.tcp.serviceName" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.tcp.serviceName.default" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.vi.callsEnabled" Type="Bool">true</Property>
		<Property Name="server.vi.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="specify.custom.address" Type="Bool">false</Property>
		<Item Name="PdynToVelocity.vi" Type="VI" URL="../PdynToVelocity.vi"/>
		<Item Name="Dependencies" Type="Dependencies"/>
		<Item Name="Build Specifications" Type="Build">
			<Item Name="Pdyn To Velocity Converter" Type="EXE">
				<Property Name="App_copyErrors" Type="Bool">true</Property>
				<Property Name="App_INI_aliasGUID" Type="Str">{0F31E3AD-28B6-45EB-8D5F-64584548D0C7}</Property>
				<Property Name="App_INI_GUID" Type="Str">{15AF0A68-F99D-477D-BE4A-6EC9488CC3A0}</Property>
				<Property Name="Bld_buildCacheID" Type="Str">{1EBCC7A6-726D-4081-9F8C-914E11C2266F}</Property>
				<Property Name="Bld_buildSpecName" Type="Str">Pdyn To Velocity Converter</Property>
				<Property Name="Bld_excludeInlineSubVIs" Type="Bool">true</Property>
				<Property Name="Bld_excludeLibraryItems" Type="Bool">true</Property>
				<Property Name="Bld_excludePolymorphicVIs" Type="Bool">true</Property>
				<Property Name="Bld_localDestDir" Type="Path">/C/Documents and Settings/Administrator/Desktop/builds/PressToVel</Property>
				<Property Name="Bld_modifyLibraryFile" Type="Bool">true</Property>
				<Property Name="Bld_previewCacheID" Type="Str">{D1F6C7D9-FFF1-4D29-A79B-E839C4DBB732}</Property>
				<Property Name="Destination[0].destName" Type="Str">PressToVel.exe</Property>
				<Property Name="Destination[0].path" Type="Path">/C/Documents and Settings/Administrator/Desktop/builds/PressToVel/PressToVel.exe</Property>
				<Property Name="Destination[0].path.type" Type="Str">&lt;none&gt;</Property>
				<Property Name="Destination[0].preserveHierarchy" Type="Bool">true</Property>
				<Property Name="Destination[0].type" Type="Str">App</Property>
				<Property Name="Destination[1].destName" Type="Str">Support Directory</Property>
				<Property Name="Destination[1].path" Type="Path">/C/Documents and Settings/Administrator/Desktop/builds/PressToVel/data</Property>
				<Property Name="Destination[1].path.type" Type="Str">&lt;none&gt;</Property>
				<Property Name="DestinationCount" Type="Int">2</Property>
				<Property Name="Source[0].itemID" Type="Str">{59F2C53D-0CB6-449F-A3FF-F7AA64CC451C}</Property>
				<Property Name="Source[0].type" Type="Str">Container</Property>
				<Property Name="Source[1].destinationIndex" Type="Int">0</Property>
				<Property Name="Source[1].itemID" Type="Ref">/My Computer/PdynToVelocity.vi</Property>
				<Property Name="Source[1].sourceInclusion" Type="Str">TopLevel</Property>
				<Property Name="Source[1].type" Type="Str">VI</Property>
				<Property Name="SourceCount" Type="Int">2</Property>
				<Property Name="TgtF_autoIncrement" Type="Bool">true</Property>
				<Property Name="TgtF_companyName" Type="Str">CPP</Property>
				<Property Name="TgtF_fileDescription" Type="Str">Pdyn To Velocity Converter</Property>
				<Property Name="TgtF_fileVersion.build" Type="Int">3</Property>
				<Property Name="TgtF_fileVersion.major" Type="Int">1</Property>
				<Property Name="TgtF_internalName" Type="Str">Pdyn To Velocity Converter</Property>
				<Property Name="TgtF_legalCopyright" Type="Str">Copyright © 2013 CPP</Property>
				<Property Name="TgtF_productName" Type="Str">Pdyn To Velocity Converter</Property>
				<Property Name="TgtF_targetfileGUID" Type="Str">{AB7A3D5D-DB57-49E6-B6B9-132BB6412FF8}</Property>
				<Property Name="TgtF_targetfileName" Type="Str">PressToVel.exe</Property>
			</Item>
		</Item>
	</Item>
</Project>
