--TPT Integrated Script Manager Korea
--The autorun to end all autoruns
--Version 3.15

--TODO:
--manual file addition (that can be anywhere and any extension)
--Moving window (because why not)
--some more API functions
--prettier, organize code

--CHANGES:
--Version 3.15: Bracket keys now scroll 5x faster, fix bracket scrolling being impossible on some keyboard layouts. Add ability to scroll by clicking and dragging, or with up/down arrow keys
--Version 3.14: Fix extra newlines being inserted into scripts on Windows
--Version 3.13: Better support for upcoming versions of TPT, all script downloads now async, settings now stored separately per scripts directory, fix another rare failure on startup
--Version 3.12: Use https for all requests, online view loads async, add FILTER button to online, fix rare failure on startup if downloaded scripts list is corrupted
--Version 3.11: Fix icons in 94.0, fix "view script in browser"
--Version 3.10: Fix HTTP requests, without this update the online section may break
--Version 3.9: Minor icon fix for latest version of jacob1's mod
--Version 3.8: Fix being unable to download scripts with / in the name, make sure tooltips don't go offscreen
--Version 3.7: Account for extra menu in TPT 91.4
--Version 3.6: Fix bug where it might delete your scripts after updating on windows
--Version 3.5: Lua5.2 support, TPT 91.0 platform API support, [] can be used to scroll, misc fixes
--Version 3.4: some new buttons, better tooltips, fix 'Change dir' button, fix broken buttons on OS X
--Version 3.3: fix apostophes in filenames, allow authors to rename their scripts on the server
--Version 3.2: put MANAGER stuff in table, fix displaying changelogs
--Version 3.1: Organize scripts less randomly, fix scripts being run twice, fix other bugs
--central script / update server at starcatcher.us / delete local scripts / lots of other things by jacob1 v3.0
--Scan all subdirectories in scripts folder! v2.25
--Remove step hooks, v87 fixes them
--Socket is now default in v87+ , horray, everyone can now use update features without extra downloads.
--Handles up to 50 extra step functions, up from the default 5 (not including the manager's step) v2.1
--Other various nice API functions
--Scripts can store/retrieve settings through the manager, see comments below v2.0
--Small fillrect change for v85, boxes can have backgrounds v1.92
--Support spaces in exe names v1.91
--Auto-update for OTHER scripts now works, is a bit messy, will fix later, but nothing should change for users to use this
--  Place a line '--VER num UPDATE link' in one of the first four lines of the file, see my above example
--  The link at top downloads a file that contains ONLY version,full link,and prints the rest(changelog). See my link for example

local icons = {
	["manager"] = "\xEE\x81\xAF",
	["cancel"] = "\xEE\x9C\x91",
	["filter"] = "\xEE\x9C\x9C",
	["search"] = "\xEE\x9C\xA1",
	["back"] = "\xEE\x9C\xAB",
	["refresh"] = "\xEE\x9C\xAC",
	["checkbox"] = "\xEE\x9C\xB9",
	["checkboxcomposite"] = "\xEE\x9C\xBA",
	["checkboxfill"] = "\xEE\x9C\xBB",
	["delete"] = "\xEE\x9D\x8D",
	["globe"] = "\xEE\x9D\xB4",
	["restart"] = "\xEE\x9D\xB7",
	["clear"] = "\xEE\xA2\x94",
	["sync"] = "\xEE\xA2\x95",
	["download"] = "\xEE\xA2\x96",
	["upload"] = "\xEE\xA2\x98",
	["openinnewwindow"] = "\xEE\xA2\xA7",
	["accept"] = "\xEE\xA3\xBB",
	["fileexplorer"] = "\xEE\xB1\x90",
	["harddrive"] = "\xEE\xB6\xA2"
}

if not socket then error("TPT version not supported") end
if MANAGER then error("manager is already running") end

local scriptversion = 17
MANAGER = {["version"] = "3.15", ["scriptversion"] = scriptversion, ["hidden"] = true}

local type = type -- people like to overwrite this function with a global a lot
local TPT_LUA_PATH = 'scripts'
local PATH_SEP = '\\'
local OS = "WIN32"
local CHECKUPDATE = false
local EXE_NAME
if platform then
	OS = platform.platform()
	if OS ~= "WIN32" and OS ~= "WIN64" then
		PATH_SEP = '/'
	end
	EXE_NAME = platform.exeName()
	local temp = EXE_NAME:reverse():find(PATH_SEP)
	EXE_NAME = EXE_NAME:sub(#EXE_NAME-temp+2)
else
	if os.getenv('HOME') then
		PATH_SEP = '/'
		if fs.exists("/Applications") then
			OS = "MACOSX"
		else
			OS = "LIN64"
		end
	end
	if OS == "WIN32" or OS == "WIN64" then
		EXE_NAME = "Powder.exe"
	elseif OS == "MACOSX" then
		EXE_NAME = "powder-x" --can't restart on OS X (if using < 91.0)
	else
		EXE_NAME = "powder"
	end
end
local beginInput, beginConfirm = ui.beginInput, ui.beginConfirm
if not beginInput then
	beginInput = function(...)
		local args = {...}
		local cb = table.remove(args)
		local input = tpt.input(unpack(args))
		cb(input)
	end
end
if not beginConfirm then
	beginConfirm = function(...)
		local args = {...}
		local cb = table.remove(args)
		local confirmed = tpt.confirm(unpack(args))
		cb(confirmed)
	end
end
local filenames = {}
local num_files = 0 --downloaded scripts aren't stored in filenames
local localscripts = {}
local onlinescripts = {}
local running = {}
local requiresrestart=false
local online = false
local first_online = true
local online_req = nil
local script_manager_update_req = nil
local updatetable --temporarily holds info on script manager updates
local gen_buttons
local count_local_scripts
local check_req_status
local sidebutton
local download_file
local settings = {}
local search_terms = {}
math.randomseed(os.time()) math.random() math.random() math.random() --some filler randoms

--get line that can be saved into scriptinfo file
local function scriptInfoString(info)
	--Write table into data format
	if type(info)~="table" then return end
	local t = {}
	for k,v in pairs(info) do
		table.insert(t,k..":\""..v.."\"")
	end
	local rstr = table.concat(t,","):gsub("\r",""):gsub("\n","\\n")
	return rstr
end

--read a scriptinfo line
local function readScriptInfo(list)
	if not list then return {} end
	local scriptlist = {}
	for i in list:gmatch("[^\n]+") do
		local t = {}
		local ID = 0
		for k,v in i:gmatch("(%w+):\"([^\"]*)\"") do
			t[k]= tonumber(v) or v:gsub("\r",""):gsub("\\n","\n")
		end
		if not t.ID then
			print("Skipping invalid script in script list")
		else
			scriptlist[t.ID] = t
		end
	end
	return scriptlist
end

--save settings
local function save_dir()
	-- Older versions of script manager stored settings here when TPT_LUA_PATH was changed away from scripts/
	-- But now, only the "DIR" argument is kept here
	fs.removeFile("autorunsettings.txt")
	if TPT_LUA_PATH ~= "scripts" then
		f = io.open("autorunsettings.txt", "wb")
		if f then
			f:write("DIR "..TPT_LUA_PATH)
			f:close()
		end
	end
end
local function save_last()
	local savestring=""
	for script,v in pairs(running) do
		savestring = savestring.." \""..script.."\""
	end
	savestring = "SAV "..savestring.."\nDIR "..TPT_LUA_PATH
	for k,t in pairs(settings) do
		for n,v in pairs(t) do
			savestring = savestring.."\nSET "..k.." "..n..":\""..v.."\""
		end
	end
	local f = io.open(TPT_LUA_PATH..PATH_SEP.."autorunsettings.txt", "wb")
	if f then
		f:write(savestring)
		f:close()
	else
		MANAGER.print("Couldn't save autorunsettings.txt")
	end

	save_dir()

	f = io.open(TPT_LUA_PATH..PATH_SEP.."downloaded"..PATH_SEP.."scriptinfo", "wb")
	if f then
		for k,v in pairs(localscripts) do
			f:write(scriptInfoString(v).."\n")
		end
		f:close()
	end
end

local function load_downloaded()
	localscripts = {}
	local f = io.open(TPT_LUA_PATH..PATH_SEP.."downloaded"..PATH_SEP.."scriptinfo","r")
	if f then
		local lines = f:read("*a")
		f:close()
		localscripts = readScriptInfo(lines)
		for k,v in pairs(localscripts) do
			if k ~= 1 then
				if not v["ID"] or not v["name"] or not v["description"] or not v["path"] or not v["version"] then
					localscripts[k] = nil
				elseif not fs.exists(TPT_LUA_PATH.."/"..v["path"]:gsub("\\","/")) then
					 localscripts[k] = nil
				end
			end
		end
	end
end

--load settings before anything else
local function load_settings(settings_file)
	local f = io.open(settings_file, "r")
	if f then
		local lines = {}
		local line = f:read("*l")
		while line do
			table.insert(lines,(line:gsub("\r","")))
			line = f:read("*l")
		end
		f:close()
		for i=1, #lines do
			local tok=lines[i]:sub(1,3)
			local str=lines[i]:sub(5)
			if tok=="SAV" then
				for word in string.gmatch(str, "\"(.-)\"") do running[word] = true end
			elseif tok=="EXE" then
				EXE_NAME=str
			elseif tok=="DIR" then
				TPT_LUA_PATH=str
			elseif tok=="SET" then
				local ident,name,val = string.match(str,"(.-) (.-):\"(.-)\"")
				if ident and name then
					if settings[ident] then settings[ident][name]=val
					else settings[ident]={[name]=val} end
				end
			end
		end
	end
end
local function load_last()
	-- Load settings from both places.
	-- Older versions of script manager may keep all settings in base-dir autorunsettings.txt
	-- Modern versions only keep the DIR arg there, and put everything else into the scripts subfolder
	load_settings("autorunsettings.txt")
	load_settings(TPT_LUA_PATH .. PATH_SEP .. "autorunsettings.txt")

	load_downloaded()
end
load_last()
--get list of files in scripts folder
local function load_filenames()
	filenames = {}
	local function searchRecursive(directory)
		local dirlist = fs.list(directory)
		if not dirlist then return end
		for i,v in ipairs(dirlist) do
			local file = directory.."/"..v
			if fs.isDirectory(file) and v ~= "downloaded" then
				searchRecursive(file)
			elseif fs.isFile(file) then
				if file:find("%.lua$") then
					local toinsert = file:sub(#TPT_LUA_PATH+2)
					if OS == "WIN32" or OS == "WIN64" then
						toinsert = toinsert:gsub("/", "\\") --not actually required
					end
					table.insert(filenames, toinsert)
				end
			end
		end
	end
	searchRecursive(TPT_LUA_PATH)
	table.sort(filenames, function(first,second) return first:lower() < second:lower() end)
end
--ui object stuff
local ui_base local ui_box local ui_line local ui_text local ui_button local ui_scrollbar local ui_tooltip local ui_checkbox local ui_console local ui_window
local tooltip
ui_base = {
new = function()
	local b={}
	b.drawlist = {}
	function b:drawadd(f)
		table.insert(self.drawlist,f)
	end
	function b:draw(...)
		for _,f in ipairs(self.drawlist) do
			if type(f)=="function" then
				f(self,...)
			end
		end
	end
	b.movelist = {}
	function b:moveadd(f)
		table.insert(self.movelist,f)
	end
	function b:onmove(x,y)
		for _,f in ipairs(self.movelist) do
			if type(f)=="function" then
				f(self,x,y)
			end
		end
	end
	return b
end
}
ui_box = {
new = function(x,y,w,h,r,g,b)
	local box=ui_base.new()
	box.x=x box.y=y box.w=w box.h=h box.x2=x+w box.y2=y+h
	box.r=r or 255 box.g=g or 255 box.b=b or 255
	function box:setcolor(r,g,b) self.r=r self.g=g self.b=b end
	function box:setbackground(r,g,b,a) self.br=r self.bg=g self.bb=b self.ba=a end
	box.drawbox=true
	box.drawbackground=false
	box:drawadd(function(self) if self.drawbackground then tpt.fillrect(self.x,self.y,self.w+1,self.h+1,self.br,self.bg,self.bb,self.ba) end
								if self.drawbox then tpt.drawrect(self.x,self.y,self.w,self.h,self.r,self.g,self.b) end end)
	box:moveadd(function(self,x,y)
		if x then self.x=self.x+x self.x2=self.x2+x end
		if y then self.y=self.y+y self.y2=self.y2+y end
	end)
	return box
end
}
ui_line = {
new=function(x,y,x2,y2,r,g,b)
	local line=ui_box.new(x,y,x2-x,y2-y,r,g,b)
	--Line is essentially a box, but with a different draw
	line.drawlist={}
	line:drawadd(function(self) tpt.drawline(self.x,self.y,self.x2,self.y2,self.r,self.g,self.b) end)
	return line
	end
}
ui_text = {
new = function(text,x,y,r,g,b)
	local txt = ui_base.new()
	txt.text = text
	txt.x=x or 0 txt.y=y or 0 txt.r=r or 255 txt.g=g or 255 txt.b=b or 255
	function txt:setcolor(r,g,b) self.r=r self.g=g self.b=b end
	txt:drawadd(function(self,x,y) tpt.drawtext(x or self.x,y or self.y,self.text,self.r,self.g,self.b) end)
	txt:moveadd(function(self,x,y)
		if x then self.x=self.x+x end
		if y then self.y=self.y+y end
	end)
	function txt:process() return false end
	return txt
end,
--Scrolls while holding mouse over
newscroll = function(text,x,y,vis,r,g,b)
	local txt = ui_text.new(text,x,y,r,g,b)
	if tpt.textwidth(text)<vis then return txt end
	txt.visible=vis
	txt.length=string.len(text)
	txt.start=1
	txt.drawlist={} --reset draw
	txt.timer=socket.gettime()+3
	function txt:cuttext(self)
		local last = self.start+1
		while tpt.textwidth(self.text:sub(self.start,last))<txt.visible and last<=self.length do
			last = last+1
		end
		self.last=last-1
	end
	txt:cuttext(txt)
	txt.minlast=txt.last-1
	txt.ppl=((txt.visible-6)/(txt.length-txt.minlast+1))
	txt:drawadd(function(self,x,y)
		if socket.gettime() > self.timer then
			if self.last >= self.length then
				self.start = 1
				self:cuttext(self)
				self.timer = socket.gettime()+3
			else
				self.start = self.start + 1
				self:cuttext(self)
				if self.last >= self.length then
					self.timer = socket.gettime()+3
				else
					self.timer = socket.gettime()+.15
				end
			end
		end
		tpt.drawtext(x or self.x,y or self.y, self.text:sub(self.start,self.last) ,self.r,self.g,self.b)
	end)
	function txt:process(mx,my,button,event,wheel)
		if event==3 then
			local newlast = math.floor((mx-self.x)/self.ppl)+self.minlast
			if newlast<self.minlast then newlast=self.minlast end
			if newlast>0 and newlast~=self.last then
				local newstart=1
				while tpt.textwidth(self.text:sub(newstart,newlast))>= self.visible do
					newstart=newstart+1
				end
				self.start=newstart self.last=newlast
				self.timer = socket.gettime()+3
			end
		end
	end
	return txt
end
}
ui_scrollbar = {
new = function(x,y,h,t,m)
	local bar = ui_base.new() --use line object as base?
	bar.x=x bar.y=y bar.h=h
	bar.total=t
	bar.numshown=m
	bar.pos=0
	bar.length=math.floor(bar.numshown / bar.total * bar.h)
	bar.soffset=math.floor(bar.pos*((bar.h-bar.length)/(bar.total-bar.numshown)))
	bar.isClicked = false
	bar.lastY = 0
	function bar:update(total,shown,pos)
		self.pos=pos or 0
		if self.pos<0 then self.pos=0 end
		self.total=total
		self.numshown=shown
		self.length= math.floor(bar.numshown / bar.total * bar.h)
		self.soffset= math.floor(self.pos*((self.h-self.length)/(self.total-self.numshown)))
	end
	function bar:move(wheel)
		self.pos = self.pos-wheel
		if self.pos < 0 then self.pos=0 end
		if self.pos > (self.total-self.numshown) then self.pos=(self.total-self.numshown) end
		self.soffset= math.floor(self.pos*((self.h-self.length)/(self.total-self.numshown)))
	end
	bar:drawadd(function(self)
		if self.total > self.numshown then
			tpt.drawline(self.x,self.y+self.soffset,self.x,self.y+self.soffset+self.length)
		end
	end)
	bar:moveadd(function(self,x,y)
		if x then self.x=self.x+x end
		if y then self.y=self.y+y end
	end)
	function bar:process(mx,my,button,event,wheel)
		if self.total <= self.numshown then
			return false
		end
		-- mousedown
		if event == 1 then
			if button == 1 then
				self.isClicked = true
				self.lastY = my
			end
		-- mouseup
		elseif event == 2 then
			self.isClicked = false
		-- mousemove (scroll items if we're dragging)
		elseif event == 3 then
			if self.isClicked then
				local diff = my - self.lastY
				-- 8 is hardcoded height of each item so it works ...
				if math.abs(diff) > 8 then
					local previous = self.pos
					if diff > 0 then
						self:move(1)
					else
						self:move(-1)
					end
					self.lastY = my
					return previous - self.pos
				end
			end
		end

		-- mousewheel (scroll items)
		if wheel~=0 and not MANAGER.hidden then
			local previous = self.pos
			self:move(wheel)
			if self.pos~=previous then
				return previous-self.pos
			end
		end
		return false
	end
	return bar
end
}
ui_button = {
new = function(x,y,w,h,f,text)
	local b = ui_box.new(x,y,w,h)
	b.f=f
	b.t=ui_text.new(text,x+3,y+5)
	b.drawbox=false
	b.clicked=false
	b.almostselected=false
	b.invert=true
	b:setbackground(127,127,127,125)
	b:drawadd(function(self)
		if self.invert and self.almostselected then
			self.almostselected=false
			tpt.fillrect(self.x,self.y,self.w,self.h)
			local tr=self.t.r local tg=self.t.g local tb=self.t.b
			b.t:setcolor(0,0,0)
			b.t:draw()
			b.t:setcolor(tr,tg,tb)
		else
			if tpt.mousex>=self.x and tpt.mousex<=self.x2 and tpt.mousey>=self.y and tpt.mousey<=self.y2 then
				self.drawbackground=true
			else
				self.drawbackground=false
			end
			b.t:draw()
		end
	end)
	b:moveadd(function(self,x,y)
		self.t:onmove(x,y)
	end)
	function b:process(mx,my,button,event,wheel)
		local clicked = self.clicked
		if event==2 then self.clicked = false end
		if mx<self.x or mx>self.x2 or my<self.y or my>self.y2 then self.clicked = false return false end
		if event==1 then
			self.clicked=true
		elseif clicked then
			if event==3 then self.almostselected=true end
			if event==2 then self:f() end
			return true
		end
	end
	return b
end
}
ui_tooltip = {
new = function(x,y,w,text)
	local b = ui_box.new(x+1,y+1,w,0)
	function b:updatetooltip(tooltip)
		self.tooltip = tooltip
		self.length = #tooltip
		self.lines = 1

		local linebreak,lastspace = 0,nil
		for i=0,#self.tooltip do
			local width = tpt.textwidth(tooltip:sub(linebreak,i+1))
			if width > self.w/2 and tooltip:sub(i,i):match("[%s,_%.%-?!]") then
				lastspace = i
			end
			local isnewline = (self.tooltip:sub(i,i) == '\n')
			if width > self.w or isnewline then
				local pos = (i==#tooltip or not lastspace) and i or lastspace
				self.lines = self.lines + 1
				if self.tooltip:sub(pos,pos) == ' ' then
					self.tooltip = self.tooltip:sub(1,pos-1).."\n"..self.tooltip:sub(pos+1)
				elseif not isnewline then
					self.length = self.length + 1
					self.tooltip = self.tooltip:sub(1,pos-1).."\n"..self.tooltip:sub(pos)
					i = i + 1
					pos = pos + 1
				end
				linebreak = pos+1
				lastspace = nil
			end
		end
		self.h = self.lines*12+2
		if self.y + self.h > gfx.HEIGHT then
			local movement = (gfx.HEIGHT-self.h-1)-self.y
			if self.y+movement < 0 then
				movement = -self.y
			end
			self:onmove(0, movement)
		end
		--self.w = tpt.textwidth(self.tooltip)+3
		self.drawbox = tooltip ~= ""
		self.drawbackground = tooltip ~= ""
	end
	function b:settooltip(tooltip_)
		tooltip:onmove(tpt.mousex+5-tooltip.x, tpt.mousey+5-tooltip.y)
		tooltip:updatetooltip(tooltip_)
	end
	b:updatetooltip(text)
	b:setbackground(0,0,0,255)
	b.drawbackground = true
	b:drawadd(function(self)
		if self.tooltip ~= "" then
			tpt.drawtext(self.x+1,self.y+2,self.tooltip)
		end
		self:updatetooltip("")
	end)
	function b:process(mx,my,button,event,wheel) end
	return b
end
}
ui_checkbox = {
up_button = function(x,y,w,h,f,text)
	local b=ui_button.new(x,y,w,h,f,text)
	b.canupdate=false
	return b
end,
new_button = function(x,y,w,h,splitx,f,f2,text,localscript)
	local b = ui_box.new(x,y,splitx,h)
	b.f=f b.f2=f2
	b.localscript=localscript
	b.splitx = splitx
	b.t=ui_text.newscroll(text,x+36,y+5,splitx-40)
	b.clicked=false
	b.selected=false
	b.checkbut=ui_checkbox.up_button(x+splitx+15, y, 15, 15, ui_button.scriptcheck, icons["sync"])
	b.drawbox=false
	b:setbackground(127,127,127,100)
	b:drawadd(function(self)
		if self.t.text == "" then return end
		self.drawbackground = false
		if tpt.mousey >= self.y and tpt.mousey < self.y+15 then
			if tpt.mousex >= self.x and tpt.mousex < self.x+15 then
				if self.localscript then
					tooltip:settooltip("이 스크립트 삭제하기")
				else
					tooltip:settooltip("브라우저에서 열기")
				end
			elseif tpt.mousex>=self.x and tpt.mousex<self.x2 then
				local script
				if online and onlinescripts[self.ID]["description"] then
					script = onlinescripts[self.ID]
				elseif not online and localscripts[self.ID] then
					script = localscripts[self.ID]
				end
				if script then
					tooltip:settooltip(script["name"].." by "..script["author"].."\n\n"..script["description"])
				end
				self.drawbackground = true
			elseif tpt.mousex >= self.x2 then
				if tpt.mousex < self.x2+9 and self.running then
					tooltip:settooltip(online and "내려받음" or "실행 중")
				elseif tpt.mousex >= self.x2+9 and tpt.mousex < self.x2+43 and self.checkbut.canupdate and onlinescripts[self.ID] and onlinescripts[self.ID]["changelog"] then
					tooltip:settooltip(onlinescripts[self.ID]["changelog"])
				end
			end
		end
		self.t:draw()
		if self.localscript then
			tpt.drawtext(self.x+3, self.y+5, icons["delete"], 255, 128, 144, 255)
		else
			tpt.drawtext(self.x+3, self.y+5, icons["openinnewwindow"], 0, 192, 255, 255)
		end
		if not self.selected then
			tpt.drawtext(self.x+18, self.y+5, icons["checkbox"])
		end
		if self.almostselected then
			self.almostselected = false
			tpt.drawtext(self.x+18, self.y+5, icons["checkboxfill"], 255, 255, 255, 192)
		elseif self.selected then
			tpt.drawtext(self.x+18, self.y+5, icons["checkboxcomposite"])
		end
		local filepath = self.ID and localscripts[self.ID] and localscripts[self.ID]["path"] or self.t.text
		if self.running then tpt.drawtext(self.x+self.splitx+3,self.y+5,online and icons["harddrive"] or "R") end
		if self.checkbut.canupdate then self.checkbut:draw() end
	end)
	b:moveadd(function(self,x,y)
		self.t:onmove(x,y)
		self.checkbut:onmove(x,y)
	end)
	function b:process(mx,my,button,event,wheel)
		if self.f2 and mx <= self.x+8 then
			if event==1 then
				self.clicked = 1
			elseif self.clicked == 1 then
				if event==3 then self.deletealmostselected = true end
				if event==2 then self:f2() end
			end
		elseif self.f and mx<=self.x+self.splitx then
			if event==1 then
				self.clicked = 2
			elseif self.clicked == 2 then
				if event==3 then self.almostselected=true end
				if event==2 then self:f() end
				self.t:process(mx,my,button,event,wheel)
			end
		else
			if self.checkbut.canupdate then self.checkbut:process(mx,my,button,event,wheel) end
			self.clicked = 0
		end
		return true
	end
	return b
end,
new = function(x,y,w,h)
	local box = ui_box.new(x,y,w,h)
	box.list={}
	box.numlist = 0
	box.max_lines = math.floor(box.h / 15) - 1
	box.max_text_width = 206
	box.splitx = x + box.max_text_width
	box.scrollbar = ui_scrollbar.new(box.x2-2,box.y+18,box.h-20,0,box.max_lines)
	box.lines={
		ui_line.new(box.x+1,box.y+16,box.x2-1,box.y+16,170,170,170),
		ui_line.new(box.x+31,box.y+16,box.x+31,box.y2-1,170,170,170),
		ui_line.new(box.splitx,box.y+16,box.splitx,box.y2-1,170,170,170),
	}
	function box:updatescroll()
		self.scrollbar:update(self.numlist,self.max_lines)
	end
	function box:clear()
		self.list={}
		self.numlist=0
	end
	function box:add(f,f2,text,localscript)
		local but = ui_checkbox.new_button(self.x,self.y+1+((self.numlist+1)*15),tpt.textwidth(text)+4,15,self.max_text_width,f,f2,text,localscript)
		table.insert(self.list,but)
		self.numlist = #self.list
		return but
	end
	box:drawadd(function (self)
		tpt.drawtext(self.x+36, self.y+4, TPT_LUA_PATH.." 폴더의 파일")
		for i,line in ipairs(self.lines) do
			line:draw()
		end
		self.scrollbar:draw()
		local restart = false
		for i,check in ipairs(self.list) do
			local filepath = check.ID and localscripts[check.ID] and localscripts[check.ID]["path"] or check.t.text
			if not check.selected and running[filepath] then
				restart = true
			end
			if i>self.scrollbar.pos and i<=self.scrollbar.pos+self.max_lines then
				check:draw()
			end
		end
		requiresrestart = restart and not online
	end)
	box:moveadd(function(self,x,y)
		for i,line in ipairs(self.lines) do
			line:onmove(x,y)
		end
		for i,check in ipairs(self.list) do
			check:onmove(x,y)
		end
	end)
	function box:scroll(amount)
		local move = amount*15
		if move==0 then return end
		for i,check in ipairs(self.list) do
			check:onmove(0,move)
			check.clicked = 0
		end
	end
	function box:process(mx,my,button,event,wheel)
		if mx<self.x or mx>self.x2 or my<self.y or my>self.y2-7 then return false end
		local scrolled = self.scrollbar:process(mx,my,button,event,wheel)
		if scrolled then self:scroll(scrolled) end
		local which = math.floor((my-self.y-11)/15)+1
		if which>0 and which<=self.numlist then self.list[which+self.scrollbar.pos]:process(mx,my,button,event,wheel) end
		if event == 2 then
			for i,v in ipairs(self.list) do v.clicked = false end
		end
		return true
	end
	return box
end
}
ui_console = {
new = function(x,y,w,h)
	local con = ui_box.new(x,y,w,h)
	con.shown_lines = math.floor(con.h/10)
	con.max_lines = 300
	con.max_width = con.w-4
	con.lines = {}
	con.scrollbar = ui_scrollbar.new(con.x2-2,con.y+1,con.h-2,0,con.shown_lines)
	con:drawadd(function(self)
		self.scrollbar:draw()
		local count=0
		for i,line in ipairs(self.lines) do
			if i>self.scrollbar.pos and i<= self.scrollbar.pos+self.shown_lines then
				line:draw(self.x+3,self.y+3+(count*10))
				count = count+1
			end
		end
	end)
	con:moveadd(function(self,x,y)
		self.scrollbar:onmove(x,y)
	end)
	function con:clear()
		self.lines = {}
		self.scrollbar:update(0,con.shown_lines)
	end
	function con:addstr(str,r,g,b)
		str = tostring(str)
		local nextl = str:find('\n')
		while nextl do
			local line = str:sub(1,nextl-1)
			self:addline(line,r,g,b)
			str = str:sub(nextl+1)
			nextl = str:find('\n')
		end
		self:addline(str,r,g,b) --anything leftover
	end
	function con:addline(line,r,g,b)
		if not line or line=="" then return end --No blank lines
		table.insert(self.lines,ui_text.newscroll(line,self.x,0,self.max_width,r,g,b))
		if #self.lines>self.max_lines then table.remove(self.lines,1) end
		self.scrollbar:update(#self.lines,self.shown_lines,#self.lines-self.shown_lines)
	end
	function con:process(mx,my,button,event,wheel)
		if mx<self.x or mx>self.x2 or my<self.y or my>self.y2 then return false end
		self.scrollbar:process(mx,my,button,event,wheel)
		local which = math.floor((my-self.y-1)/10)+1
		if which>0 and which<=self.shown_lines and self.lines[which+self.scrollbar.pos] then self.lines[which+self.scrollbar.pos]:process(mx,my,button,event,wheel) end
		return true
	end
	return con
end
}
ui_window = {
new = function(x,y,w,h)
	local w=ui_box.new(x,y,w,h)
	w.sub={}
	function w:add(m,name)
		if name then w[name]=m end
		table.insert(self.sub,m)
		return m
	end
	w:drawadd(function(self)
		for i,sub in ipairs(self.sub) do
			sub:draw()
		end
	end)
	w:moveadd(function(self,x,y)
		for i,sub in ipairs(self.sub) do
			sub:onmove(x,y)
		end
	end)
	function w:process(mx,my,button,event,wheel)
		if (mx<self.x or mx>self.x2 or my<self.y or my>self.y2) and event == 1 then ui_button.sidepressed() return true end
		local ret
		for i,sub in ipairs(self.sub) do
			if sub:process(mx,my,button,event,wheel) then ret = true end
		end
		return ret
	end
	return w
end
}
--Main window with everything!
local mainwindow = ui_window.new(57,51,513,321)
mainwindow:setbackground(10,10,10,235) mainwindow.drawbackground=true
mainwindow:add(ui_console.new(297,87,273,285),"menuconsole")
mainwindow:add(ui_checkbox.new(57,71,240,301),"checkbox")
tooltip = ui_tooltip.new(0,1,250,"")

--Some API functions you can call from other scripts
--put 'using_manager=MANAGER ~= nil' or similar in your scripts, using_manager will be true if the manager is active
--Print a message to the manager console, can be colored
function MANAGER.print(msg,...)
	mainwindow.menuconsole:addstr(msg,...)
end
-- Gets script info table for a script, or all scripts if nil is used as id. Data is fetched from the server.
-- Returns table as argument to callback function once info download finishes, or nil and http status code if download / parsing failed
function MANAGER.scriptinfo(id, callback)
	if not callback then error("Callback function argument is required") end

	local url = "https://starcatcher.us/scripts/main.lua"
	if id then
		url = url.."?info="..id
	end
	download_file(url, function(info, status_code)
		if status_code == 200 then
			local infotable = readScriptInfo(info)
			callback(id and infotable[id] or infotable)
		else
			callback(nil, status_code)
		end
	end)
end
--Get various info about the system (operating system, script directory, path seperator, if socket is loaded)
function MANAGER.sysinfo()
	return {["OS"]=OS, ["scriptDir"]=TPT_LUA_PATH, ["pathSep"]=PATH_SEP, ["exeName"] = EXE_NAME}
end
--Save a setting in the autorun settings file, ident should be your script name no one else would use.
--Name is variable name, val is the value which will be saved/returned as a string
function MANAGER.savesetting(ident,name,val)
	ident = tostring(ident)
	name = tostring(name)
	val = tostring(val)
	if settings[ident] then settings[ident][name]=val
	else settings[ident]={[name]=val} end
	save_last()
end
--Get a previously saved value, if it has one
function MANAGER.getsetting(ident,name)
	if settings[ident] then return settings[ident][name] end
	return nil
end
--delete a setting, leave name nil to delete all of ident
function MANAGER.delsetting(ident,name)
	if settings[ident] then
	if name then settings[ident][name]=nil
	else settings[ident]=nil end
	save_last()
	end
end

local active_downloads = {}
function download_file(url, cb)
	if not http then
		MANAGER.print("TPT 95.0 or greater required to use http api", 255, 0, 0)
		return false
	end
	if not cb then
		MANAGER.print("Callback function required for async download", 255, 0, 0)
		return false
	end
	local req = http.get(url)
	local timeout_after = socket.gettime() + 3
	table.insert(active_downloads, {req=req, timeout_after=timeout_after, cb=cb})
end

local function process_downloads()
	for k,v in pairs(active_downloads) do
		local req = v["req"]
		local cb = v["cb"]
		local timeout_after = v["timeout_after"]

		local status = req:status()
		if status ~= "running" then
			active_downloads[k] = nil
			local body, status_code = req:finish()
			if status_code and status_code ~= 200 then
				MANAGER.print("http download failed with status code " .. status_code, 255, 0, 0)
			end
			cb(body, status_code)
		end

		if socket.gettime() > timeout_after then
			active_downloads[k] = nil
			MANAGER.print("http download timed out ", 255, 0, 0)
			req:cancel()
			cb(nil, 408)
		end
	end
end

--Downloads script to a location, runs callback function with either true or false argument indicading success
local function download_script(ID, location, cb)
	download_file("https://starcatcher.us/scripts/main.lua?get=" .. ID, function(file, status_code)
		if file and status_code == 200 then
			f = io.open(location, "wb")
			f:write(file)
			f:close()
			cb(true, status_code)
		else
			MANAGER.print("Got http status " .. status_code .. " while downloading script", 255, 0, 0)
			cb(false, status_code)
		end
	end)
end

--Restart exe (if named correctly)
local function do_restart(skip_save)
	if not skip_save then
		save_last()
	end
	if platform then
		platform.restart()
	end
	if OS == "WIN32" or OS == "WIN64" then
		os.execute("TASKKILL /IM \""..EXE_NAME.."\" /F &&START .\\\""..EXE_NAME.."\"")
	elseif OS == "OSX" then
		MANAGER.print("Can't restart on macOS when using game versions less than 91.0, please manually close and reopen The Powder Toy")
		return
	else
		os.execute("killall -s KILL \""..EXE_NAME.."\" && ./\""..EXE_NAME.."\"")
	end
	MANAGER.print("Restart failed, do you have the exe name right?",255,0,0)
end
local function open_link(url)
	if platform then
		platform.openLink(url)
	else
		local command = (OS == "WIN32" or OS == "WIN64") and "start" or (OS == "MACOSX" and "open" or "xdg-open")
		os.execute(command.." "..url)
	end
end
--TPT interface
local function step()
	tpt.fillrect(-1,-1,gfx.WIDTH,gfx.HEIGHT,0,0,0,150)
	mainwindow:draw()
	tpt.drawtext(80, 58, "스크립트 관리자* " ..MANAGER.version)
	if requiresrestart then
		tpt.drawtext(302, 58, "스크립트를 비활성화하려면 다시 시작해야 합니다.", 255, 50, 50)
	else
		tpt.drawtext(282, 58, "스크립트를 클릭하여 토글하고 ".. (online and icons["download"] or icons["accept"]) .. " 단추를 누르십시오.")
	end
	tpt.drawtext(302,75,"콘솔 출력:")
	tooltip:draw()

	if online_req and online then
		local textwidth = tpt.textwidth("불러오는 중...")
		tpt.drawtext(mainwindow.checkbox.x + (mainwindow.checkbox.w - textwidth) / 2, mainwindow.checkbox.y + (mainwindow.checkbox.h - 6) / 2, "불러오는 중...")
	end
end
local function mouseclick(mousex,mousey,button,event,wheel)
	sidebutton:process(mousex,mousey,button,event,wheel)
	if MANAGER.hidden then return true end

	if mousex>612 or mousey>384 then return false end
	mainwindow:process(mousex,mousey,button,event,wheel)
	return false
end
local function keypress(key, scan, rep, shift, ctrl, alt)
	if jacobsmod and (scan == ui.SDL_SCANCODE_O) and not rep then jacobsmod_old_menu_check = true end
	if scan == ui.SDL_SCANCODE_ESCAPE and not MANAGER.hidden then MANAGER.hidden=true return false end
	if MANAGER.hidden then return end

	if scan == ui.SDL_SCANCODE_LEFTBRACKET or scan == ui.SDL_SCANCODE_UP then
		mainwindow:process(mainwindow.x+30, mainwindow.y+30, 0, 2, 5)
	elseif scan == ui.SDL_SCANCODE_RIGHTBRACKET or scan == ui.SDL_SCANCODE_DOWN then
		mainwindow:process(mainwindow.x+30, mainwindow.y+30, 0, 2, -5)
	end

	return false
end
--small button on right to bring up main menu
local WHITE = {255,255,255,255}
local BLACK = {0,0,0,255}
local function smallstep()
	gfx.drawRect(sidebutton.x, sidebutton.y+1, sidebutton.w+1, sidebutton.h+1,200,200,200)
	local color=WHITE
	if not MANAGER.hidden then
		step()
		gfx.fillRect(sidebutton.x, sidebutton.y+1, sidebutton.w+1, sidebutton.h+1)
		color=BLACK
	end
	tpt.drawtext(sidebutton.x+2, sidebutton.y+4, icons["manager"], color[1], color[2], color[3], 255)
	check_req_status()
end
local function reload_action()
	load_filenames()
	load_downloaded()
	if not online then
		gen_buttons()
		mainwindow.checkbox:updatescroll()
	else
		count_local_scripts()
	end
	if num_files == 0 then
		MANAGER.print("No scripts found in '"..TPT_LUA_PATH.."' folder",255,255,0)
		fs.makeDirectory(TPT_LUA_PATH)
	else
		MANAGER.print("Reloaded file list, found "..num_files.." scripts")
	end
end
--button functions on click
function ui_button.reloadpressed(self)
	if not online then
		reload_action()
	else
		beginInput("스크립트 검색", "스크립트를 검색합니다.", function(filter)
			search_terms = {}
			for match in filter:gmatch("%w+") do
				table.insert(search_terms, match)
			end
			gen_buttons()
		end)
	end
end
function ui_button.selectnone(self)
	for i,but in ipairs(mainwindow.checkbox.list) do
		but.selected = false
	end
end
function ui_button.consoleclear(self)
	mainwindow.menuconsole:clear()
end
function ui_button.changedir(self)
	beginInput("검색할 디렉터리 변경", "스크립트와 설정이 있는 폴더를 입력하십시오(다시 시작해야 함).", TPT_LUA_PATH, TPT_LUA_PATH, function(new)
		local last = TPT_LUA_PATH
		if new and new~=last and new~="" then
			save_last()

			MANAGER.print("Directory changed to "..new,255,255,0)
			TPT_LUA_PATH = new

			save_dir()
			do_restart(true)
		end
	end)
end
function ui_button.uploadscript(self)
	if not online then
		local command = (OS == "WIN32" or OS == "WIN64") and "start" or (OS == "MACOSX" and "open" or "xdg-open")
		os.execute(command.." "..TPT_LUA_PATH)
	else
		open_link("https://starcatcher.us/scripts/paste.lua")
	end
end
local lastpaused
function ui_button.sidepressed(self)
	if TPTMP and TPTMP.chatHidden == false then print("Minimize Multiplay Manager before opening the Script Manager") return end
	MANAGER.hidden = not MANAGER.hidden
	ui_button.localview()
	if not MANAGER.hidden then
		lastpaused = tpt.set_pause()
		tpt.set_pause(1)
		ui_button.reloadpressed()
	else
		tpt.set_pause(lastpaused)
	end
end
function ui_button.donepressed(self)
	MANAGER.hidden = true
	for i,but in ipairs(mainwindow.checkbox.list) do
		local filepath = but.ID and localscripts[but.ID]["path"] or but.t.text
		if but.selected then
			if requiresrestart then
				running[filepath] = true
			else
				if not running[filepath] then
					local status,err = pcall(dofile,TPT_LUA_PATH..PATH_SEP..filepath)
					if not status then
						MANAGER.print(err,255,0,0)
						print(err)
						but.selected = false
					else
						MANAGER.print("Started "..filepath)
						running[filepath] = true
					end
				end
			end
		elseif running[filepath] then
			running[filepath] = nil
		end
	end
	if requiresrestart then do_restart() return end
	save_last()
end
function ui_button.downloadpressed(self)
	local remaining_downloads = 0
	local any_failed = false
	for i,but in ipairs(mainwindow.checkbox.list) do
		if but.selected then
			local displayName = "downloaded"..PATH_SEP..but.ID.." "..onlinescripts[but.ID].author:gsub("[^%w _-]", "_").."-"..onlinescripts[but.ID].name:gsub("[^%w _-]", "_")..".lua"
			local name = TPT_LUA_PATH..PATH_SEP..displayName
			if not fs.exists(TPT_LUA_PATH..PATH_SEP.."downloaded") then
				fs.makeDirectory(TPT_LUA_PATH..PATH_SEP.."downloaded")
			end

			remaining_downloads = remaining_downloads + 1
			download_script(but.ID, name, function(success, status_code)
				if success then
					local status, err = pcall(function()
						if localscripts[but.ID] and localscripts[but.ID]["path"] ~= displayName then
							local oldpath = localscripts[but.ID]["path"]
							fs.removeFile(TPT_LUA_PATH.."/"..oldpath:gsub("\\","/"))
							running[oldpath] = nil
						end
						localscripts[but.ID] = onlinescripts[but.ID]
						localscripts[but.ID]["path"] = displayName
						dofile(name)

						MANAGER.print("Downloaded and started "..but.t.text)
						running[displayName] = true
					end)
					if not status then
						MANAGER.print(err)
						any_failed = true
					end
				else
					any_failed = true
				end

				-- All remaining downloads finished, close the manager and save data
				remaining_downloads = remaining_downloads - 1
				if remaining_downloads == 0 then
					save_last()
					print("Finished downloading and installing scripts")
					if not any_failed then
						MANAGER.hidden = true
						ui_button.localview()
					else
						print("Some scripts failed, see manager log")
					end
				end
			end)
		end
	end
end

function ui_button.pressed(self)
	self.selected = not self.selected
end
function ui_button.delete(self)
	local cb = function(confirmed)
		if confirmed ~= true then return end

		local filepath = self.ID and localscripts[self.ID]["path"] or self.t.text
		fs.removeFile(TPT_LUA_PATH.."/"..filepath:gsub("\\","/"))
		if running[filepath] then running[filepath] = nil end
		if localscripts[self.ID] then localscripts[self.ID] = nil end
		save_last()
		ui_button.localview()
		load_filenames()
		gen_buttons()
	end
	beginConfirm("파일 제거", self.t.text.."을(를) 제거하시겠습니까?", cb)
end
function ui_button.viewonline(self)
	open_link("https://starcatcher.us/scripts?view="..self.ID)
end
function ui_button.scriptcheck(self)
	local oldpath = localscripts[self.ID]["path"]
	local newpath = "downloaded"..PATH_SEP..self.ID.." "..onlinescripts[self.ID].author:gsub("[^%w _-]", "_").."-"..onlinescripts[self.ID].name:gsub("[^%w _-]", "_")..".lua"
	download_script(self.ID, TPT_LUA_PATH..PATH_SEP..newpath, function(success, status_code)
		if not success then return end

		self.canupdate = false
		localscripts[self.ID] = onlinescripts[self.ID]
		localscripts[self.ID]["path"] = newpath
		if oldpath:gsub("\\","/") ~= newpath:gsub("\\","/") then
			fs.removeFile(TPT_LUA_PATH.."/"..oldpath:gsub("\\","/"))
			if running[oldpath] then
				running[newpath],running[oldpath] = running[oldpath],nil
			end
		end
		if running[newpath] then
			do_restart()
		else
			save_last()
			MANAGER.print(onlinescripts[self.ID]["name"].."를 업데이트함")
		end
	end)
end
function ui_button.doupdate(self)
	fileSystem.move("autorun.lua", "autorunold.lua")
	download_script(1, "autorun.lua", function()
		localscripts[1] = updatetable[1]
		do_restart()
	end)
end
local globebutton
local donebutton
local uploadscriptbutton, reloadbutton
function ui_button.localview(self)
	if online then
		online = false
		gen_buttons()
		globebutton.t.text = icons["globe"]
		globebutton.f = ui_button.onlineview
		donebutton.t.text = (requiresrestart and icons["restart"] or icons["accept"])
		donebutton.t.x = 538
		donebutton.x = 535
		donebutton.x2 = 550
		donebutton.f = ui_button.donepressed
		uploadscriptbutton.t.text = icons["fileexplorer"]
		reloadbutton.t.text = icons["refresh"]
	end
end
function ui_button.onlineview(self)
	if not online then
		online = true
		gen_buttons()
		globebutton.t.text = icons["harddrive"]
		globebutton.f = ui_button.localview
		donebutton.t.text = icons["download"]
		donebutton.t.x = 262
		donebutton.x = 259
		donebutton.x2 = 274
		donebutton.f = ui_button.downloadpressed
		uploadscriptbutton.t.text = icons["upload"]
		reloadbutton.t.text = icons["filter"]
		search_terms = {}
	end
end
--add buttons to window
mainwindow:add(ui_button.new(59, 53, 15, 15, ui_button.sidepressed, icons["back"]))
globebutton = ui_button.new(191, 53, 15, 15, ui_button.onlineview, icons["globe"])
mainwindow:add(globebutton)
reloadbutton = ui_button.new(208, 53, 15, 15, ui_button.reloadpressed, icons["refresh"])
mainwindow:add(reloadbutton)
uploadscriptbutton = ui_button.new(225, 53, 15, 15, ui_button.uploadscript, icons["fileexplorer"])
mainwindow:add(uploadscriptbutton)
mainwindow:add(ui_button.new(242, 53, 15, 15, ui_button.changedir, icons["search"]))
donebutton = ui_button.new(535, 53, 15, 15, ui_button.donepressed, icons["accept"])
mainwindow:add(donebutton)
mainwindow:add(ui_button.new(552, 53, 15, 15, ui_button.sidepressed, icons["cancel"]))
local nonebutton = ui_button.new(75, 74, 10, 10,ui_button.selectnone,"")
nonebutton.drawbox = true
mainwindow:add(nonebutton)
mainwindow:add(ui_button.new(552, 69, 15, 15, ui_button.consoleclear,icons["clear"]))
local ypos = 134
if tpt.num_menus then
	ypos = 390-16*tpt.num_menus()-(16 or 0)
end
sidebutton = ui_button.new(gfx.WIDTH-16,gfx.HEIGHT-305,14,14,ui_button.sidepressed,'')

local function gen_buttons_local()
	local count = 0
	local sorted = {}
	for k,v in pairs(localscripts) do if v.ID ~= 1 then table.insert(sorted, v) end end
	table.sort(sorted, function(first,second) return first.name:lower() < second.name:lower() end)
	for i,v in ipairs(sorted) do
		local check = mainwindow.checkbox:add(ui_button.pressed,ui_button.delete,v.name,true)
		check.ID = v.ID
		if running[v.path] then
			check.running = true
			check.selected = true
		end
		count = count + 1
	end
	if #sorted >= 5 and #filenames >= 5 then
		mainwindow.checkbox:add(nil, nil, "", false) --empty space to separate things
	end
	for i=1,#filenames do
		local check = mainwindow.checkbox:add(ui_button.pressed,ui_button.delete,filenames[i],true)
		if running[filenames[i]] then
			check.running = true
			check.selected = true
		end
	end
	num_files = count + #filenames
end
function count_local_scripts()
	local total = #filenames
	for k in pairs(localscripts) do total = total + 1 end
	num_files = total
end
local function gen_buttons_online()
	if not http then
		MANAGER.print("TPT 95.0 or greater required to use the online tab", 255, 0, 0)
		return
	end

	if online_req then
		online_req:cancel()
	end

	online_req = http.get("https://starcatcher.us/scripts/main.lua")

	if first_online then
		first_online = false
		script_manager_update_req = http.get("https://starcatcher.us/scripts/main.lua?info=1")
	end
end

local function check_search_term(script, search_term)
	search_term = search_term:lower()
	return script.name and script.name:lower():match(search_term)
		or script.description and script.description:lower():match(search_term)
		or script.author and script.author:lower():match(search_term)
end

-- Check status of "Online" tab request
local function check_online_req_status()
	if online_req and online_req:status() ~= "running" then
		local list, status_code = online_req:finish()
		online_req = nil
		if status_code ~= 200 then
			MANAGER.print("script list download failed with status code " .. status_code, 255, 0, 0)
			return
		end

		if not online then return end

		onlinescripts = readScriptInfo(list)
		local sorted = {}
		for k,v in pairs(onlinescripts) do table.insert(sorted, v) end
		table.sort(sorted, function(first,second) return first.ID < second.ID end)
		for k,v in pairs(sorted) do
			local matches = true
			for i,term in ipairs(search_terms) do
				matches = matches and check_search_term(v, term)
			end
			if matches then
				local check = mainwindow.checkbox:add(ui_button.pressed, ui_button.viewonline, v.name, false)
				check.ID = v.ID
				check.checkbut.ID = v.ID
				if localscripts[v.ID] then
					check.running = true
					if tonumber(v.version) > tonumber(localscripts[check.ID].version) then
						check.checkbut.canupdate = true
					end
				end
			end
		end

		mainwindow.checkbox:updatescroll()
	end
end

-- Check status of self update check
local function check_update_req_status()
	if script_manager_update_req and script_manager_update_req:status() ~= "running" then
		local updateinfo, status_code = script_manager_update_req:finish()
		script_manager_update_req = nil
		if status_code ~= 200 then
			MANAGER.print("self update check failed with status code " .. status_code, 255, 0, 0)
			return
		end

		updatetable = readScriptInfo(updateinfo)
		if not updatetable[1] then return end
		if tonumber(updatetable[1].version) > scriptversion then
			local updatebutton = ui_button.new(276,53,110,15,ui_button.doupdate,icons["sync"].." 업데이트 사용 가능")
			updatebutton.t:setcolor(25,255,25)
			mainwindow:add(updatebutton)
			MANAGER.print("A script manager update is available! Click UPDATE",25,255,55)
			MANAGER.print(updatetable[1].changelog,25,255,55)
		end
	end
end

-- Check status of pending requests
function check_req_status()
	check_online_req_status()
	check_update_req_status()
	-- Check other misc downloads (script or update downloads)
	process_downloads()
end

gen_buttons = function()
	mainwindow.checkbox:clear()
	if online then
		gen_buttons_online()
	else
		gen_buttons_local()
	end
	mainwindow.checkbox:updatescroll()
end
gen_buttons()

--register manager first
tpt.register_step(smallstep)
--load previously running scripts
local started = ""
for prev,v in pairs(running) do
	local status,err = pcall(dofile,TPT_LUA_PATH..PATH_SEP..prev)
	if not status then
		MANAGER.print(err,255,0,0)
		running[prev] = nil
	else
		started=started.." "..prev
		local newbut = mainwindow.checkbox:add(ui_button.pressed,prev,nil,false)
		newbut.selected=true
	end
end
save_last()
if started~="" then
	MANAGER.print("Auto started"..started)
end
tpt.register_mouseevent(mouseclick)
evt.register(evt.keypress, keypress)
