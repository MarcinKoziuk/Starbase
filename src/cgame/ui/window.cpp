#pragma once

#include <tb/tb_node_tree.h>
#include <tb/tb_widgets_reader.h>

#include <starbase/game/logging.hpp>
#include <starbase/cgame/ui/window.hpp>

namespace Starbase {
namespace UI {

Window::Window(tb::TBWidget& root)
{
	root.AddChild(this);
}

Window::~Window()
{
	tb::TBWidget* parent = GetParent();

	if (parent != nullptr)
		parent->RemoveChild(this);
}

bool Window::LoadResourceFile(const char *filename)
{
	// We could do g_widgets_reader->LoadFile(this, filename) but we want
	// some extra data we store under "WindowInfo", so read into node tree.
	tb::TBNode node;
	if (!node.ReadFile(filename)) {
		LOG(error) << "Could not load layout file " << filename;
		return false;
	}

	LoadResource(node);
	return true;
}

void Window::LoadResource(tb::TBNode &node)
{
	tb::g_widgets_reader->LoadNodeTree(this, &node);

	// Get title from the WindowInfo section (or use "" if not specified)
	const char* title = node.GetValueString("WindowInfo>title", "");
	SetText(title);

	const tb::TBRect parent_rect(0, 0, GetParent()->GetRect().w, GetParent()->GetRect().h);
	const tb::TBDimensionConverter *dc = tb::g_tb_skin->GetDimensionConverter();
	tb::TBRect window_rect = GetResizeToFitContentRect();

	// Use specified size or adapt to the preferred content size.
	tb::TBNode *tmp = node.GetNode("WindowInfo>size");
	if (tmp && tmp->GetValue().GetArrayLength() == 2)
	{
		window_rect.w = dc->GetPxFromString(tmp->GetValue().GetArray()->GetValue(0)->GetString(), window_rect.w);
		window_rect.h = dc->GetPxFromString(tmp->GetValue().GetArray()->GetValue(1)->GetString(), window_rect.h);
	}

	// Use the specified position or center in parent.
	tmp = node.GetNode("WindowInfo>position");
	if (tmp && tmp->GetValue().GetArrayLength() == 2)
	{
		window_rect.x = dc->GetPxFromString(tmp->GetValue().GetArray()->GetValue(0)->GetString(), window_rect.x);
		window_rect.y = dc->GetPxFromString(tmp->GetValue().GetArray()->GetValue(1)->GetString(), window_rect.y);
	}
	else
		window_rect = window_rect.CenterIn(parent_rect);

	// Make sure the window is inside the parent, and not larger.
	window_rect = window_rect.MoveIn(parent_rect).Clip(parent_rect);

	SetRect(window_rect);

	// Ensure we have focus - now that we've filled the window with possible focusable
	// widgets. EnsureFocus was automatically called when the window was activated (by
	// adding the window to the root), but then we had nothing to focus.
	// Alternatively, we could add the window after setting it up properly.
	EnsureFocus();
}

} // namespace UI
} // namespace Starbase