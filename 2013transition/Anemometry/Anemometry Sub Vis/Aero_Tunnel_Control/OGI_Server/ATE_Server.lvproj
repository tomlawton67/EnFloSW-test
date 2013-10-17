<?xml version='1.0' encoding='UTF-8'?>
<Project Type="Project" LVVersion="11008008">
	<Property Name="NI.LV.All.SourceOnly" Type="Bool">true</Property>
	<Property Name="varPersistentID:{18716088-B5D5-4B7D-99A6-FCB9F4747A7F}" Type="Ref">/My Computer/ATE_Shared_Variables.lvlib/Fan_Data</Property>
	<Property Name="varPersistentID:{7864FEB8-1664-47B4-AE03-9119E945373E}" Type="Ref">/My Computer/ATE_Shared_Variables.lvlib/Road_Control</Property>
	<Property Name="varPersistentID:{78DDADEA-37BC-4719-B411-5612128C6F6A}" Type="Ref">/My Computer/ATE_Shared_Variables.lvlib/Road_Data</Property>
	<Property Name="varPersistentID:{C2F513FE-8A15-411A-BA2D-B4A3F8C492FD}" Type="Ref">/My Computer/ATE_Shared_Variables.lvlib/Fan_Control</Property>
	<Property Name="varPersistentID:{CE2CF7A7-238E-4EA7-9B6E-18FB210B5842}" Type="Ref">/My Computer/ATE_Shared_Variables.lvlib/Balance_Data</Property>
	<Property Name="varPersistentID:{FDDAD6A4-08E2-4A11-9807-6E599FC8ABAF}" Type="Ref">/My Computer/ATE_Shared_Variables.lvlib/ATE_Machine_Status</Property>
	<Item Name="My Computer" Type="My Computer">
		<Property Name="NI.SortType" Type="Int">3</Property>
		<Property Name="server.app.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.control.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.tcp.enabled" Type="Bool">false</Property>
		<Property Name="server.tcp.port" Type="Int">0</Property>
		<Property Name="server.tcp.serviceName" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.tcp.serviceName.default" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.vi.callsEnabled" Type="Bool">true</Property>
		<Property Name="server.vi.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="specify.custom.address" Type="Bool">false</Property>
		<Item Name="Controls" Type="Folder">
			<Item Name="ATE_Balance_Data.ctl" Type="VI" URL="../ATE_Balance_Data.ctl"/>
			<Item Name="ATE_Fan_Control.ctl" Type="VI" URL="../ATE_Fan_Control.ctl"/>
			<Item Name="ATE_Fan_Data.ctl" Type="VI" URL="../ATE_Fan_Data.ctl"/>
			<Item Name="ATE_Machine_Status.ctl" Type="VI" URL="../ATE_Machine_Status.ctl"/>
			<Item Name="ATE_Road_Control.ctl" Type="VI" URL="../ATE_Road_Control.ctl"/>
			<Item Name="ATE_Road_Data.ctl" Type="VI" URL="../ATE_Road_Data.ctl"/>
		</Item>
		<Item Name="ATE_Shared_Variables.lvlib" Type="Library" URL="../ATE_Shared_Variables.lvlib"/>
		<Item Name="Server.vi" Type="VI" URL="../Server.vi"/>
		<Item Name="Aero_Tunnel_Monitor.vi" Type="VI" URL="../Aero_Tunnel_Monitor.vi"/>
		<Item Name="Dependencies" Type="Dependencies">
			<Item Name="vi.lib" Type="Folder">
				<Item Name="DialogType.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/DialogType.ctl"/>
				<Item Name="LV70TimeStampToDateRec.vi" Type="VI" URL="/&lt;vilib&gt;/_oldvers/_oldvers.llb/LV70TimeStampToDateRec.vi"/>
				<Item Name="Check if File or Folder Exists.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/libraryn.llb/Check if File or Folder Exists.vi"/>
				<Item Name="NI_FileType.lvlib" Type="Library" URL="/&lt;vilib&gt;/Utility/lvfile.llb/NI_FileType.lvlib"/>
				<Item Name="Error Cluster From Error Code.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Error Cluster From Error Code.vi"/>
				<Item Name="whitespace.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/whitespace.ctl"/>
				<Item Name="Trim Whitespace.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Trim Whitespace.vi"/>
				<Item Name="NI_PackedLibraryUtility.lvlib" Type="Library" URL="/&lt;vilib&gt;/Utility/LVLibp/NI_PackedLibraryUtility.lvlib"/>
				<Item Name="Simple Error Handler.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Simple Error Handler.vi"/>
				<Item Name="General Error Handler.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/General Error Handler.vi"/>
				<Item Name="DialogTypeEnum.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/DialogTypeEnum.ctl"/>
				<Item Name="General Error Handler CORE.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/General Error Handler CORE.vi"/>
				<Item Name="Check Special Tags.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Check Special Tags.vi"/>
				<Item Name="TagReturnType.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/TagReturnType.ctl"/>
				<Item Name="Set String Value.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Set String Value.vi"/>
				<Item Name="GetRTHostConnectedProp.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/GetRTHostConnectedProp.vi"/>
				<Item Name="Error Code Database.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Error Code Database.vi"/>
				<Item Name="Format Message String.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Format Message String.vi"/>
				<Item Name="Find Tag.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Find Tag.vi"/>
				<Item Name="Search and Replace Pattern.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Search and Replace Pattern.vi"/>
				<Item Name="Set Bold Text.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Set Bold Text.vi"/>
				<Item Name="Details Display Dialog.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Details Display Dialog.vi"/>
				<Item Name="ErrWarn.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/ErrWarn.ctl"/>
				<Item Name="eventvkey.ctl" Type="VI" URL="/&lt;vilib&gt;/event_ctls.llb/eventvkey.ctl"/>
				<Item Name="Clear Errors.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Clear Errors.vi"/>
				<Item Name="Not Found Dialog.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Not Found Dialog.vi"/>
				<Item Name="Three Button Dialog.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Three Button Dialog.vi"/>
				<Item Name="Three Button Dialog CORE.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Three Button Dialog CORE.vi"/>
				<Item Name="Longest Line Length in Pixels.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Longest Line Length in Pixels.vi"/>
				<Item Name="Convert property node font to graphics font.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Convert property node font to graphics font.vi"/>
				<Item Name="Get Text Rect.vi" Type="VI" URL="/&lt;vilib&gt;/picture/picture.llb/Get Text Rect.vi"/>
				<Item Name="Get String Text Bounds.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Get String Text Bounds.vi"/>
				<Item Name="LVBoundsTypeDef.ctl" Type="VI" URL="/&lt;vilib&gt;/Utility/miscctls.llb/LVBoundsTypeDef.ctl"/>
				<Item Name="BuildHelpPath.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/BuildHelpPath.vi"/>
				<Item Name="GetHelpDir.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/GetHelpDir.vi"/>
				<Item Name="Write File+ (string).vi" Type="VI" URL="/&lt;vilib&gt;/Utility/file.llb/Write File+ (string).vi"/>
				<Item Name="compatWriteText.vi" Type="VI" URL="/&lt;vilib&gt;/_oldvers/_oldvers.llb/compatWriteText.vi"/>
				<Item Name="Close File+.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/file.llb/Close File+.vi"/>
				<Item Name="Find First Error.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/error.llb/Find First Error.vi"/>
				<Item Name="compatCalcOffset.vi" Type="VI" URL="/&lt;vilib&gt;/_oldvers/_oldvers.llb/compatCalcOffset.vi"/>
				<Item Name="Read File+ (string).vi" Type="VI" URL="/&lt;vilib&gt;/Utility/file.llb/Read File+ (string).vi"/>
				<Item Name="compatReadText.vi" Type="VI" URL="/&lt;vilib&gt;/_oldvers/_oldvers.llb/compatReadText.vi"/>
				<Item Name="Rendezvous RefNum" Type="VI" URL="/&lt;vilib&gt;/Utility/rendezvs.llb/Rendezvous RefNum"/>
				<Item Name="Open File+.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/file.llb/Open File+.vi"/>
				<Item Name="Space Constant.vi" Type="VI" URL="/&lt;vilib&gt;/dlg_ctls.llb/Space Constant.vi"/>
				<Item Name="System Exec.vi" Type="VI" URL="/&lt;vilib&gt;/Platform/system.llb/System Exec.vi"/>
				<Item Name="NI_SMTPEmail.lvlib" Type="Library" URL="/&lt;vilib&gt;/Utility/SMTP/NI_SMTPEmail.lvlib"/>
				<Item Name="LV70U32ToDateRec.vi" Type="VI" URL="/&lt;vilib&gt;/_oldvers/_oldvers.llb/LV70U32ToDateRec.vi"/>
				<Item Name="GetDateTimeInSecsCompatVI.vi" Type="VI" URL="/&lt;vilib&gt;/_oldvers/_oldvers.llb/GetDateTimeInSecsCompatVI.vi"/>
				<Item Name="Compare Two Paths.vi" Type="VI" URL="/&lt;vilib&gt;/Utility/libraryn.llb/Compare Two Paths.vi"/>
				<Item Name="Open URL in Default Browser core.vi" Type="VI" URL="/&lt;vilib&gt;/Platform/browser.llb/Open URL in Default Browser core.vi"/>
				<Item Name="Open URL in Default Browser (string).vi" Type="VI" URL="/&lt;vilib&gt;/Platform/browser.llb/Open URL in Default Browser (string).vi"/>
				<Item Name="Escape Characters for HTTP.vi" Type="VI" URL="/&lt;vilib&gt;/printing/PathToURL.llb/Escape Characters for HTTP.vi"/>
				<Item Name="Path to URL.vi" Type="VI" URL="/&lt;vilib&gt;/printing/PathToURL.llb/Path to URL.vi"/>
				<Item Name="Open URL in Default Browser (path).vi" Type="VI" URL="/&lt;vilib&gt;/Platform/browser.llb/Open URL in Default Browser (path).vi"/>
				<Item Name="Open URL in Default Browser.vi" Type="VI" URL="/&lt;vilib&gt;/Platform/browser.llb/Open URL in Default Browser.vi"/>
			</Item>
			<Item Name="user.lib" Type="Folder">
				<Item Name="DtLvLink.lvlib" Type="Library" URL="/&lt;userlib&gt;/LV-Link3/DtLvLink.lvlib"/>
				<Item Name="DtOLWriteSingle.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtOLWriteSingle.vi"/>
				<Item Name="DtOLWriteWaveform.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtOLWriteWaveform.vi"/>
				<Item Name="DtOLWriteMultiple2D.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtOLWriteMultiple2D.vi"/>
				<Item Name="DtOLWriteMultiple1D.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtOLWriteMultiple1D.vi"/>
				<Item Name="DtOLWrite.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/DtOLWrite.vi"/>
				<Item Name="DtOLCloseTask.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/DtOLCloseTask.vi"/>
				<Item Name="DtCreateAoutChan.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtCreateAoutChan.vi"/>
				<Item Name="DtCreateQuadChan.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtCreateQuadChan.vi"/>
				<Item Name="DtCreateDOutChan.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtCreateDOutChan.vi"/>
				<Item Name="DtCreateDinChan.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtCreateDinChan.vi"/>
				<Item Name="DtCreateCtrContMeasureChan.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtCreateCtrContMeasureChan.vi"/>
				<Item Name="DtCreateCtrTwoEdgeChan.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtCreateCtrTwoEdgeChan.vi"/>
				<Item Name="DtCreateCtrPeriodChan.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtCreateCtrPeriodChan.vi"/>
				<Item Name="DtCreateCtrFreqChan.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtCreateCtrFreqChan.vi"/>
				<Item Name="DtCreateCtrUpDownChan.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtCreateCtrUpDownChan.vi"/>
				<Item Name="DtCreateCtrRateChan.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtCreateCtrRateChan.vi"/>
				<Item Name="DtCreateCtrOneShotRepeatChan.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtCreateCtrOneShotRepeatChan.vi"/>
				<Item Name="DtCreateCtrOneShotChan.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtCreateCtrOneShotChan.vi"/>
				<Item Name="DtCreateCtrEventsChan.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtCreateCtrEventsChan.vi"/>
				<Item Name="DtCreateAoutRawChan.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtCreateAoutRawChan.vi"/>
				<Item Name="DtCreateAinRawChan.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtCreateAinRawChan.vi"/>
				<Item Name="DtCreateAinChan.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/PolySubVIs/DtCreateAinChan.vi"/>
				<Item Name="DtOLCreateTask.vi" Type="VI" URL="/&lt;userlib&gt;/LV-Link3/DtOLCreateTask.vi"/>
			</Item>
			<Item Name="ADAM_Module_Type.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/ADAM_Module_Type.vi"/>
			<Item Name="ADAM_setup_ch_names.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/ADAM_setup_ch_names.vi"/>
			<Item Name="Add_filename_suffix.vi" Type="VI" URL="../../../File/Add_filename_suffix.vi"/>
			<Item Name="Add_filename_to_err_message.vi" Type="VI" URL="../../../Utilities/Add_filename_to_err_message.vi"/>
			<Item Name="Add_label_to_string.vi" Type="VI" URL="../../../String/Add_label_to_string.vi"/>
			<Item Name="Address.ctl" Type="VI" URL="../../../../../ADAM Client/Sub VIs/ADAM Controls/Address.ctl"/>
			<Item Name="Analyse_string_array.vi" Type="VI" URL="../../../String/Analyse_string_array.vi"/>
			<Item Name="Analysis_info_cluster.ctl" Type="VI" URL="../../../Controls/Analysis_info_cluster.ctl"/>
			<Item Name="Analysis_type_cluster.ctl" Type="VI" URL="../../../Controls/Analysis_type_cluster.ctl"/>
			<Item Name="Analysis_type_Enum.ctl" Type="VI" URL="../../../Controls/Analysis_type_Enum.ctl"/>
			<Item Name="App_Global.ctl" Type="VI" URL="../../../Controls/App_Global.ctl"/>
			<Item Name="App_Ref_Cluster.ctl" Type="VI" URL="../../../Controls/App_Ref_Cluster.ctl"/>
			<Item Name="App_Ref_Global.vi" Type="VI" URL="../../../Globals/App_Ref_Global.vi"/>
			<Item Name="ASYNC_Adam_client.vi" Type="VI" URL="../../../../../ADAM Client/ADAM client VIs/ASYNC_Adam_client.vi"/>
			<Item Name="Board_Types.ctl" Type="VI" URL="../../../Controls/Board_Types.ctl"/>
			<Item Name="Build_Pathname.vi" Type="VI" URL="../../../File/Build_Pathname.vi"/>
			<Item Name="Calib_type_ADAM.ctl" Type="VI" URL="../../../../../ADAM Client/Sub VIs/ADAM Controls/Calib_type_ADAM.ctl"/>
			<Item Name="Can_File_be_Editted.vi" Type="VI" URL="../../../File/Can_File_be_Editted.vi"/>
			<Item Name="Ch_and_Spans.ctl" Type="VI" URL="../../../Controls/Ch_and_Spans.ctl"/>
			<Item Name="Ch_description_ADAM.ctl" Type="VI" URL="../../../../../ADAM Client/Sub VIs/ADAM Controls/Ch_description_ADAM.ctl"/>
			<Item Name="Ch_dis_to_Str_ADAM.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/Ch_dis_to_Str_ADAM.vi"/>
			<Item Name="Ch_names_to_rows.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/Ch_names_to_rows.vi"/>
			<Item Name="Channel_name_checker.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/Channel_name_checker.vi"/>
			<Item Name="Channel_State.ctl" Type="VI" URL="../../../../../ADAM Client/Sub VIs/ADAM Controls/Channel_State.ctl"/>
			<Item Name="Characters_to_string.vi" Type="VI" URL="../../../String/Characters_to_string.vi"/>
			<Item Name="Check_and_rewrite_file.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/Check_and_rewrite_file.vi"/>
			<Item Name="Check_file_header.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/Check_file_header.vi"/>
			<Item Name="Constants_global.vi" Type="VI" URL="../../../Globals/Constants_global.vi"/>
			<Item Name="Convert_No._to_String_ADAM.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/Convert_No._to_String_ADAM.vi"/>
			<Item Name="Copy_with_Check.vi" Type="VI" URL="../../../File/Copy_with_Check.vi"/>
			<Item Name="Create_directory_path.vi" Type="VI" URL="../../../File/Create_directory_path.vi"/>
			<Item Name="Create_Error_log_folders.vi" Type="VI" URL="../../../Utilities/Create_Error_log_folders.vi"/>
			<Item Name="Date_and_Time_to_string.vi" Type="VI" URL="../../../String/Date_and_Time_to_string.vi"/>
			<Item Name="Display_message.vi" Type="VI" URL="../../../Display_message.vi"/>
			<Item Name="Does_file_exist_(simple).vi" Type="VI" URL="../../../File/Does_file_exist_(simple).vi"/>
			<Item Name="Dynamic_call_global.ph.vi" Type="VI" URL="../../../Globals/Dynamic_call_global.ph.vi"/>
			<Item Name="Email_or_SMS.vi" Type="VI" URL="../../../Utilities/Email_or_SMS.vi"/>
			<Item Name="Email_to_rcpt.vi" Type="VI" URL="../../../Utilities/Email_to_rcpt.vi"/>
			<Item Name="Email_transfer_global.vi" Type="VI" URL="../../../Utilities/Email_transfer_global.vi"/>
			<Item Name="EnFlo_Data_Folder_Setup.ctl" Type="VI" URL="../../../Controls/EnFlo_Data_Folder_Setup.ctl"/>
			<Item Name="EnFlo_Error_Handler.vi" Type="VI" URL="../../../Utilities/EnFlo_Error_Handler.vi"/>
			<Item Name="Fast_string_Replace.vi" Type="VI" URL="../../../String/Fast_string_Replace.vi"/>
			<Item Name="File_Modified_Since_Check.vi" Type="VI" URL="../../../File/File_Modified_Since_Check.vi"/>
			<Item Name="File_Status.ctl" Type="VI" URL="../../../File/File_Status.ctl"/>
			<Item Name="Files_Unable_to_Update.vi" Type="VI" URL="../../../File/Files_Unable_to_Update.vi"/>
			<Item Name="Full_Client_Name.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/Full_Client_Name.vi"/>
			<Item Name="Function_ADAM.ctl" Type="VI" URL="../../../../../ADAM Client/Sub VIs/ADAM Controls/Function_ADAM.ctl"/>
			<Item Name="Get_array_values_fr_String.vi" Type="VI" URL="../../../String/Get_array_values_fr_String.vi"/>
			<Item Name="Get_Call_chain.vi" Type="VI" URL="../../../File/Get_Call_chain.vi"/>
			<Item Name="Get_Current_Scan_client.vi" Type="VI" URL="../../../../../ADAM Client/ADAM client VIs/Get_Current_Scan_client.vi"/>
			<Item Name="Get_IP_Number.vi" Type="VI" URL="../../../Utilities/Get_IP_Number.vi"/>
			<Item Name="Get_link_info.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/Get_link_info.vi"/>
			<Item Name="Get_string_subset.vi" Type="VI" URL="../../../String/Get_string_subset.vi"/>
			<Item Name="Global_mode.ctl" Type="VI" URL="../../../Controls/Global_mode.ctl"/>
			<Item Name="Graph_Type.ctl" Type="VI" URL="../../../Controls/Graph_Type.ctl"/>
			<Item Name="Inst_type.ctl" Type="VI" URL="../../../Controls/Inst_type.ctl"/>
			<Item Name="Instrument_Array.ctl" Type="VI" URL="../../../Controls/Instrument_Array.ctl"/>
			<Item Name="Launch_Async_Email.vi" Type="VI" URL="../../../Utilities/Launch_Async_Email.vi"/>
			<Item Name="Meas_global.vi" Type="VI" URL="../../../Globals/Meas_global.vi"/>
			<Item Name="Merge_channel_names.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/Merge_channel_names.vi"/>
			<Item Name="Module_type_ADAM.ctl" Type="VI" URL="../../../../../ADAM Client/Sub VIs/ADAM Controls/Module_type_ADAM.ctl"/>
			<Item Name="Multi_run_mode.ctl" Type="VI" URL="../../../Controls/Multi_run_mode.ctl"/>
			<Item Name="Number_2_string.vi" Type="VI" URL="../../../String/Number_2_string.vi"/>
			<Item Name="Open_File_for_Read.vi" Type="VI" URL="../../../File/Open_File_for_Read.vi"/>
			<Item Name="Open_File_for_write.vi" Type="VI" URL="../../../File/Open_File_for_write.vi"/>
			<Item Name="Open_File_for_write_check.vi" Type="VI" URL="../../../File/Open_File_for_write_check.vi"/>
			<Item Name="Overhead.ctl" Type="VI" URL="../../../Controls/Overhead.ctl"/>
			<Item Name="Overhead_Factor.vi" Type="VI" URL="../../../Gen Trav/Overhead_Factor.vi"/>
			<Item Name="Path_2_String.vi" Type="VI" URL="../../../File/Path_2_String.vi"/>
			<Item Name="Polynomial_convert.vi" Type="VI" URL="../../../Calibration/Polynomial_convert.vi"/>
			<Item Name="Post_Process.ctl" Type="VI" URL="../../../Controls/Post_Process.ctl"/>
			<Item Name="Preference_global.vi" Type="VI" URL="../../../Globals/Preference_global.vi"/>
			<Item Name="Program_Active_Aborting.vi" Type="VI" URL="../../../../../ADAM Client/ADAM client VIs/Program_Active_Aborting.vi"/>
			<Item Name="PWA_Data.ctl" Type="VI" URL="../../../Controls/PWA_Data.ctl"/>
			<Item Name="Read_Ch_data_file.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/Read_Ch_data_file.vi"/>
			<Item Name="Read_Characters_From_File.ph.vi" Type="VI" URL="../../../File/Read_Characters_From_File.ph.vi"/>
			<Item Name="Read_File_No_Error.vi" Type="VI" URL="../../../File/Read_File_No_Error.vi"/>
			<Item Name="Read_Write_Local_table.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/Read_Write_Local_table.vi"/>
			<Item Name="Remove_unprintable_Char.vi" Type="VI" URL="../../../String/Remove_unprintable_Char.vi"/>
			<Item Name="Replace_Character.vi" Type="VI" URL="../../../String/Replace_Character.vi"/>
			<Item Name="Reverse_filepath.vi" Type="VI" URL="../../../File/Reverse_filepath.vi"/>
			<Item Name="Row_type_selector_string.vi" Type="VI" URL="../../../Gen Trav/Row_type_selector_string.vi"/>
			<Item Name="Send_SMS.vi" Type="VI" URL="../../../Utilities/Send_SMS.vi"/>
			<Item Name="Served_Global.vi" Type="VI" URL="../../../Globals/Served_Global.vi"/>
			<Item Name="Serveds.ctl" Type="VI" URL="../../../Controls/Serveds.ctl"/>
			<Item Name="Server_Comms.ctl" Type="VI" URL="../../../Controls/Server_Comms.ctl"/>
			<Item Name="Setup_Filepath.vi" Type="VI" URL="../../../File/Setup_Filepath.vi"/>
			<Item Name="Setup_Filetype.ctl" Type="VI" URL="../../../File/Setup_Filetype.ctl"/>
			<Item Name="SMS_to_mobile.vi" Type="VI" URL="../../../Utilities/SMS_to_mobile.vi"/>
			<Item Name="Spacing.ctl" Type="VI" URL="../../../Gen Trav/Spacing.ctl"/>
			<Item Name="Split_at_return.vi" Type="VI" URL="../../../String/Split_at_return.vi"/>
			<Item Name="Split_Ch_name.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/Split_Ch_name.vi"/>
			<Item Name="Start_Client_Coms.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/Start_Client_Coms.vi"/>
			<Item Name="Str_to_Ch_dis_ADAM.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/Str_to_Ch_dis_ADAM.vi"/>
			<Item Name="String_to_Ch_names.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/String_to_Ch_names.vi"/>
			<Item Name="String_to_Pathname.vi" Type="VI" URL="../../../File/String_to_Pathname.vi"/>
			<Item Name="TCP_email_comms.vi" Type="VI" URL="../../../Utilities/TCP_email_comms.vi"/>
			<Item Name="Test_server_port.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/Test_server_port.vi"/>
			<Item Name="Text_without_format.vi" Type="VI" URL="../../../String/Text_without_format.vi"/>
			<Item Name="Timed_Error_dialog.vi" Type="VI" URL="../../../Utilities/Timed_Error_dialog.vi"/>
			<Item Name="Trav_pos_for_Email.vi" Type="VI" URL="../../../Gen Trav/Trav_pos_for_Email.vi"/>
			<Item Name="Trav_pos_global.vi" Type="VI" URL="../../../Globals/Trav_pos_global.vi"/>
			<Item Name="Traverse_cluster.ctl" Type="VI" URL="../../../Gen Trav/Traverse_cluster.ctl"/>
			<Item Name="Traverse_Level_State.ctl" Type="VI" URL="../../../../../ADAM Client/Sub VIs/ADAM Controls/Traverse_Level_State.ctl"/>
			<Item Name="Trigger_Mode.ctl" Type="VI" URL="../../../Controls/Trigger_Mode.ctl"/>
			<Item Name="Try_Updating_Files.vi" Type="VI" URL="../../../File/Try_Updating_Files.vi"/>
			<Item Name="Tunnel_Coordinates.ctl" Type="VI" URL="../../../Controls/Tunnel_Coordinates.ctl"/>
			<Item Name="Update_Current_Value_client.vi" Type="VI" URL="../../../../../ADAM Client/ADAM client VIs/Update_Current_Value_client.vi"/>
			<Item Name="Update_scan_value.ctl" Type="VI" URL="../../../../../ADAM Client/Sub VIs/ADAM Controls/Update_scan_value.ctl"/>
			<Item Name="Updated_File_Copy.vi" Type="VI" URL="../../../File/Updated_File_Copy.vi"/>
			<Item Name="Upload_To_ADAM_Server.vi" Type="VI" URL="../../../../../ADAM Client/Sub VIs/Upload_To_ADAM_Server.vi"/>
			<Item Name="Valid_Message.vi" Type="VI" URL="../../../Utilities/Valid_Message.vi"/>
			<Item Name="Valid_Path.vi" Type="VI" URL="../../../File/Valid_Path.vi"/>
			<Item Name="Wait_seconds.vi" Type="VI" URL="../../../Wait_seconds.vi"/>
			<Item Name="Was_string_matched.vi" Type="VI" URL="../../../String/Was_string_matched.vi"/>
			<Item Name="Write_File_Simple.vi" Type="VI" URL="../../../File/Write_File_Simple.vi"/>
			<Item Name="XY_Plot_Cluster.ctl" Type="VI" URL="../../../Controls/XY_Plot_Cluster.ctl"/>
			<Item Name="Correlation_cluster1.ctl" Type="VI" URL="../../../Controls/Correlation_cluster1.ctl"/>
			<Item Name="XDNodeRunTimeDep.lvlib" Type="Library" URL="/&lt;vilib&gt;/Platform/TimedLoop/XDataNode/XDNodeRunTimeDep.lvlib"/>
			<Item Name="Open_Close_Front_Panel.ph.vi" Type="VI" URL="../../../Utilities/Open_Close_Front_Panel.ph.vi"/>
			<Item Name="Run_EnFlo_Hub.vi" Type="VI" URL="../../../../Utilities/Run_EnFlo_Hub.vi"/>
		</Item>
		<Item Name="Build Specifications" Type="Build"/>
	</Item>
</Project>
