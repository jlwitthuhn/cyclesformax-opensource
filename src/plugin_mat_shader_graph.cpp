/* 
 * This file is part of Cycles for Max. (c) Jeffrey Witthuhn
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
 
#include "plugin_mat_shader_graph.h"

#include <atomic>
#include <chrono>
#include <fstream>
#include <mutex>
#include <sstream>
#include <thread>

#include <3dsmaxdlport.h>
#include <iparamm2.h>
#include <IPathConfigMgr.h>
#include <Shobjidl.h>

#include <shader_editor/shader_editor.h>
#include <shader_graph/graph.h>

#include "rend_shader_desc.h"
#include "win_resource.h"

extern HINSTANCE hInstance;

static const USHORT SERIALIZED_GRAPH_CHUNK = 1234;

SHADER_PARAMS_ENUMS

static bool shader_graph_window_open = false;

static std::wstring wstring_from_string(std::string input) {
	std::wstringstream stream;
	for (char c : input) {
		stream << static_cast<wchar_t>(c);
	}
	return stream.str();
}

static std::string string_from_wstr(WStr input) {
	std::stringstream stream;
	for (int i = 0; i < input.length(); i++) {
		if (input[i] < 256) {
			stream << static_cast<char>(input[i]);
		}
		else {
			stream << '?';
		}
	}
	return stream.str();
}

class ShaderEditorSharedData {
public:
	bool input_updated();
	bool output_updated();

	std::string get_input_graph();
	void set_input_graph(const std::string& new_graph);

	std::string get_output_graph();
	void set_output_graph(const std::string& new_graph);

	void request_stop() { return stop.store(true); }
	bool should_stop() { return stop.load(); }

private:
	std::mutex input_mutex;
	std::string input_graph;
	bool _input_updated{ false };

	std::mutex output_mutex;
	std::string output_graph;
	bool _output_updated{ false };

	std::atomic<bool> stop{ false };
};

bool ShaderEditorSharedData::input_updated()
{
	std::lock_guard<std::mutex> lock(input_mutex);
	const bool result{ _input_updated };
	return result;
}

bool ShaderEditorSharedData::output_updated()
{
	std::lock_guard<std::mutex> lock(output_mutex);
	const bool result{ _output_updated };
	return result;
}

std::string ShaderEditorSharedData::get_input_graph()
{
	std::lock_guard<std::mutex> lock(input_mutex);
	const std::string result{ input_graph };
	_input_updated = false;
	return result;
}

void ShaderEditorSharedData::set_input_graph(const std::string& new_graph)
{
	std::lock_guard<std::mutex> lock(input_mutex);
	input_graph = new_graph;
	_input_updated = true;
}

std::string ShaderEditorSharedData::get_output_graph()
{
	std::lock_guard<std::mutex> lock(output_mutex);
	const std::string result{ output_graph };
	_output_updated = false;
	return result;
}

void ShaderEditorSharedData::set_output_graph(const std::string& new_graph)
{
	std::lock_guard<std::mutex> lock(output_mutex);
	output_graph = new_graph;
	_output_updated = true;
}

static void shader_editor_thread_func(std::shared_ptr<ShaderEditorSharedData> shared_data)
{
	cse::ShaderGraphEditor editor;
	editor.open_window();
	while (editor.running()) {
		if (shared_data->should_stop()) {
			editor.force_close();
			break;
		}
		if (shared_data->input_updated()) {
			editor.load_graph(shared_data->get_input_graph());
		}
		if (editor.has_new_data()) {
			shared_data->set_output_graph(editor.get_serialized_graph());
		}

		std::this_thread::sleep_for(std::chrono::milliseconds{ 50 });
	}

	shader_graph_window_open = false;
}

static void SaveShaderFile(std::string graph)
{
	// Nonsense for file type selection
	COMDLG_FILTERSPEC rg_spec[] =
	{
		{ L"Shader File", L"*.shader" },
		{ L"Text File", L"*.txt" },
	};

	MaxSDK::Util::Path project_path = IPathConfigMgr::GetPathConfigMgr()->GetCurrentProjectFolder();
	const MCHAR* const cwstr_path = project_path.GetCStr();

	IShellItem* directory_item = nullptr;
	const HRESULT hr = SHCreateItemFromParsingName(cwstr_path, nullptr, IID_PPV_ARGS(&directory_item));

	if (SUCCEEDED(hr)) {
		IFileDialog* file_dlg = nullptr;
		const HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&file_dlg));

		if (SUCCEEDED(hr)) {
			file_dlg->SetFolder(directory_item);
			file_dlg->SetFileName(L"shader.shader");
			file_dlg->SetFileTypes(2, rg_spec);

			const HRESULT hr = file_dlg->Show(nullptr);
			if (SUCCEEDED(hr)) {
				IShellItem* save_item = nullptr;
				const HRESULT hr = file_dlg->GetResult(&save_item);

				if (SUCCEEDED(hr)) {
					wchar_t* wstr_path = new wchar_t[MAX_PATH];

					const HRESULT hr = save_item->GetDisplayName(SIGDN_FILESYSPATH, &wstr_path);

					if (SUCCEEDED(hr)) {
						std::ofstream file_stream(wstr_path, std::ofstream::trunc);
						file_stream << graph;
						file_stream.close();
					}
					delete[] wstr_path;
					save_item->Release();
				}
			}
			file_dlg->Release();
		}
	}

	if (directory_item != nullptr) {
		directory_item->Release();
	}
}

static std::string OpenShaderFile()
{
	// Nonsense for file type selection
	COMDLG_FILTERSPEC rg_spec[] =
	{
		{ L"Shader File", L"*.shader" },
		{ L"Text File", L"*.txt" },
	};

	std::stringstream result_stream;

	MaxSDK::Util::Path project_path = IPathConfigMgr::GetPathConfigMgr()->GetCurrentProjectFolder();
	const MCHAR* cwstr_path = project_path.GetCStr();

	IShellItem* directory_item = nullptr;
	HRESULT hr = SHCreateItemFromParsingName(cwstr_path, nullptr, IID_PPV_ARGS(&directory_item));

	if (SUCCEEDED(hr)) {
		IFileDialog* file_dlg = nullptr;
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&file_dlg));

		if (SUCCEEDED(hr)) {
			hr = file_dlg->SetFolder(directory_item);
			hr = file_dlg->SetFileTypes(2, rg_spec);
			hr = file_dlg->Show(nullptr);

			if (SUCCEEDED(hr)) {
				IShellItem* shell_item = nullptr;
				hr = file_dlg->GetResult(&shell_item);

				if (SUCCEEDED(hr)) {
					PWSTR file_path;
					hr = shell_item->GetDisplayName(SIGDN_FILESYSPATH, &file_path);

					if (SUCCEEDED(hr)) {
						std::ifstream input_file(file_path, std::ifstream::in);

						int c = input_file.get();

						while (input_file.good()) {
							if (c != EOF) {
								result_stream << static_cast<char>(c);
							}
							c = input_file.get();
						}
					}
					shell_item->Release();
				}
			}
			file_dlg->Release();
		}
	}

	if (directory_item != nullptr) {
		directory_item->Release();
	}

	return result_stream.str();
}

static INT_PTR RollupDlgProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	CyclesShaderGraphMatBase* mat_ptr = DLGetWindowLongPtr<CyclesShaderGraphMatBase*>(hWnd);

	if (msg == WM_INITDIALOG) {
		mat_ptr = reinterpret_cast<CyclesShaderGraphMatBase*>(lParam);
		DLSetWindowLongPtr(hWnd, lParam);
		return TRUE;
	}
	else if (msg == WM_COMMAND) {
		if (LOWORD(wParam) == IDC_BUTTON_OPEN_NODE_EDITOR) {
			mat_ptr->OpenNodeEditor();
			return TRUE;
		}
		else if (LOWORD(wParam) == IDC_BUTTON_UPDATE_PREVIEW) {
			mat_ptr->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}
		else if (LOWORD(wParam) == IDC_BUTTON_SAVE_SHADER) {
			std::string graph = mat_ptr->GetNodeGraph();
			SaveShaderFile(graph);
		}
		else if (LOWORD(wParam) == IDC_BUTTON_LOAD_SHADER) {
			std::string graph = OpenShaderFile();
			if (graph != "") {
				mat_ptr->loaded_graph = graph;
				mat_ptr->loaded_graph_set = true;
			}
		}
	}

	return FALSE;
}

CyclesShaderGraphMatBase::CyclesShaderGraphMatBase() : CyclesPluginMatBase{ false, false }, shared_data{ std::make_shared<ShaderEditorSharedData>() }
{
	node_type = ClosureType::SPECIAL;

	color_diffuse = Color(0.8f, 0.8f, 0.8f);

	// Default graph
	loaded_graph = "cycles_shader|1|section_nodes|out_material|default_output|0|0|node_end|section_connections|";
	loaded_graph_set = true;
}

void CyclesShaderGraphMatBase::OpenNodeEditor()
{
	if (loaded_graph_set == true) {
		shared_data->set_input_graph(loaded_graph);
		shared_data->set_output_graph(loaded_graph);
		shared_data->get_output_graph(); // Reset the 'new' flag
		loaded_graph_set = false;
	}
	if (shader_graph_window_open) {
		return;
	}
	else {
		if (shader_editor_thread.joinable()) {
			shader_editor_thread.join();
		}
		shader_graph_window_open = true;
		shared_data->set_input_graph(GetNodeGraph());
		shader_editor_thread = std::thread(shader_editor_thread_func, shared_data);
	}
}

std::string CyclesShaderGraphMatBase::GetNodeGraph()
{
	if (shared_data == nullptr) {
		return std::string();
	}

	if (loaded_graph_set == true) {
		shared_data->set_input_graph(loaded_graph);
		shared_data->set_output_graph(loaded_graph);
		loaded_graph_set = false;
	}

	return shared_data->get_output_graph();
}

void CyclesShaderGraphMatBase::PopulateShaderParamsDesc(ShaderParamsDescriptor* const desc)
{
	IParamBlock2* const params_pblock = GetShaderParamsPblock();
	if (params_pblock == nullptr) {
		return;
	}

	desc->use_mis = (params_pblock->GetInt(param_shader_surface_use_mis) != 0);
	desc->displacement = static_cast<DisplacementMethod>(params_pblock->GetInt(param_shader_surface_displacement));
	desc->vol_sampling = static_cast<VolumeSamplingMethod>(params_pblock->GetInt(param_shader_vol_sampling));
	desc->vol_interp = static_cast<VolumeInterpolationMethod>(params_pblock->GetInt(param_shader_vol_interp));
	desc->vol_is_homogeneous = (params_pblock->GetInt(param_shader_vol_homogeneous) != 0);
}

ParamDlg* CyclesShaderGraphMatBase::CreateParamDlg(const HWND hwMtlEdit, IMtlParams* const imp)
{
	imp->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL_MAT_SHADER_GRAPH),
		RollupDlgProc,
		_T("Node Graph"),
		(LPARAM)this);

	IAutoMParamDlg* const master_dlg = dynamic_cast<IAutoMParamDlg*>(CyclesPluginMatBase::CreateParamDlg(hwMtlEdit, imp));

	return master_dlg;
}

IOResult CyclesShaderGraphMatBase::Load(ILoad* const iload)
{
	while (iload->OpenChunk() == IO_OK) {

		if (iload->CurChunkID() == SERIALIZED_GRAPH_CHUNK) {
			char* chunk_buffer = nullptr;
			IOResult read_result = iload->ReadCStringChunk(&chunk_buffer);
			if (read_result == IO_OK && chunk_buffer != nullptr) {
				std::string str_graph(chunk_buffer);
				loaded_graph = str_graph;
				loaded_graph_set = true;
			}
		}
		iload->CloseChunk();
	}
	return IO_OK;
}

IOResult CyclesShaderGraphMatBase::Save(ISave* const isave)
{
	std::string serialized_graph;
	if (loaded_graph_set) {
		serialized_graph = loaded_graph;
	}
	else {
		serialized_graph = shared_data->get_output_graph();
	}

	isave->BeginChunk(SERIALIZED_GRAPH_CHUNK);
	isave->WriteCString(serialized_graph.c_str());
	isave->EndChunk();

	return IO_OK;
}

void CyclesShaderGraphMatBase::Update(const TimeValue t, Interval& valid)
{
	if (pblock == nullptr) {
		return;
	}

	if (t >= valid_interval.Start() && t <= valid_interval.End()) {
		valid &= valid_interval;
		return;
	}

	Interval subthings_valid = FOREVER;
	UpdateSubThings(t, subthings_valid);

	valid_interval = subthings_valid;
	valid &= valid_interval;

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

BaseInterface* CyclesShaderGraphMatBase::GetInterface(Interface_ID id)
{
	if (id == SHADER_GRAPH_OPS_ID) {
		return dynamic_cast<ShaderGraphOpsMixin*>(this);
	}
	else {
		return CyclesPluginMatBase::GetInterface(id);
	}
}

TSTR CyclesShaderGraphMatBase::GetShaderGraph()
{
	return WStr{ wstring_from_string(GetNodeGraph()).c_str() };
}

void CyclesShaderGraphMatBase::SetShaderGraph(MSTR input)
{
	const std::string input_string = string_from_wstr(input);
	static boost::optional<csg::Graph> maybe_graph = csg::Graph::from(input_string);
	if (maybe_graph && maybe_graph->nodes().size() > 0) {
		loaded_graph_set = false;
		shared_data->set_input_graph(string_from_wstr(input));
		shared_data->set_output_graph(string_from_wstr(input));
	}
}
