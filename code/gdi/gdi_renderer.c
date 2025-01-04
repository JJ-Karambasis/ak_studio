function gdi_render_cmd* GDI_Allocate_Cmd(gdi_renderer* Renderer) {
	Assert(Renderer->CmdCount < GDI_MAX_RENDER_CMD_COUNT);
	gdi_render_cmd* Cmd = Renderer->Cmds + Renderer->CmdCount++;
	return Cmd;
}

function void GDI_Draw_Rect(gdi_renderer* Renderer, v2 Min, v2 Max, v4 Color) {
	gdi_render_cmd* Cmd = GDI_Allocate_Cmd(Renderer);
	Cmd->Type = GDI_RENDER_CMD_TYPE_DRAW_RECT;
	Cmd->DrawRect.Min = Min;
	Cmd->DrawRect.Max = Max;
	Cmd->DrawRect.Color = Color;
}