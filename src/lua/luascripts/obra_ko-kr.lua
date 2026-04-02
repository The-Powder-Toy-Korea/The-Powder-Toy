local ptMax = 0 -- Max particle type number in use
local visibleNum = 0 -- Number of visible elements
-- For vanilla ptMax = 193 (including NONE = 0), visibleNum = 172 (20 hidden elements)

-- TODO: Clear all elements to the default state, in case mods are enabled
elem.loadDefault()

-- Compute maximum element number used
for t = 1, sim.PT_NUM do
	if elements.exists(t) and t > ptMax then
		ptMax = t
	end
end

local cipherFactor = 113 -- a number used to scramble elements, should be coprime to ptMax+1
local correctInfo = {}
local decipher = {}
local unlockedElements = {}
local elemExists = {}
local elemVisible = {}
local guesses = {}
local amountHidden = 0 -- scrambled elements
local amountMenuHidden = 0 -- scrambled elements that are in menus
local userUnlockedVisible = false
local unlockedSomething = false -- did user unlock at least one element?
local savedNotes = {}

for t = 1, ptMax do
	elemExists[t] = elements.exists(t)

	if elemExists[t] then
		elemVisible[t] = elements.property(t, "MenuVisible") ~= 0
		if elemVisible[t] then
			visibleNum = visibleNum + 1
		end
	end
end

-- By default none of the elements are unlocked
for t = 1, ptMax do
	unlockedElements[t] = false

	-- Save correct info
	if elemExists[t] then
		correctInfo[elements.property(t, "Name")] = t
	end
end
-- ... besides life
unlockedElements[elements.DEFAULT_PT_LIFE] = true

local function cipher(t)
	return tostring((t*cipherFactor) % (ptMax+1))
end

-- Scramble elements
local scramble = function ()
	for t = 1, ptMax do
		if elemExists[t] and not unlockedElements[t] then
			local sections = {elements.SC_ELEC, elements.SC_POWERED, elements.SC_SENSOR, elements.SC_FORCE, elements.SC_EXPLOSIVE, elements.SC_GAS,
			elements.SC_LIQUID, elements.SC_POWDERS, elements.SC_SOLIDS, elements.SC_NUCLEAR, elements.SC_SPECIAL }

			-- Use existing guess, otherwise use cipher
			local ciphernbr = cipher(t)
			local ciphertext = guesses[t] and guesses[t] or ciphernbr
			decipher[ciphertext] = t

			-- Table used for hiding identifying information
			local col = 20 + (ciphernbr % 230)
			hiddenInfo = {
				Name = ciphertext,
				MenuSection = sections[1 + (ciphernbr % 11)],
				MenuSort = ciphernbr,
				Description = savedNotes[t] and savedNotes[t] or "???",
				Color = gfx.getHexColor(col, col, col),
			}

			-- Clear graphics function too
			if t ~= elements.DEFAULT_PT_SOAP then
				hiddenInfo["Graphics"] = function() return 1 end
			end

			elem.element(t, hiddenInfo)
			amountHidden = amountHidden + 1
			if elemVisible[t] then
				amountMenuHidden = amountMenuHidden + 1
			end
		end
	end
end

-- Check which elements were guessed correctly
local checkElements = function ()
	local correct = {}
	local correctNum = 0

	for t, v in pairs(guesses) do
		if correctInfo[elements.property(t, "Name")] == t then
			table.insert(correct, t)
			correctNum = correctNum + 1
		end
	end

	-- If guessed correctly, restore all info
	local correctNames = ""
	local firstTime = false

	if correctNum >= 3 or (correctNum >= 1 and amountMenuHidden < 3) -- When less than 3 visible elements left, solve one at a time
		or (correctNum >= 1 and not unlockedSomething) -- Accept the first correct guess
	then
		for _, t in ipairs(correct) do
			unlockedElements[t] = true
			guesses[t] = nil
			elem.loadDefault(t)
			elements.property(t, "Name", elements.property(t, "Name")) -- A workaround for the menu bug
			correctNames = correctNames .. elements.property(t, "Name") .. ", "
			amountHidden = amountHidden - 1

			if elemVisible[t] then
				amountMenuHidden = amountMenuHidden - 1
			end

			-- Show rules when the user guesses something correctly for the first time
			if not unlockedSomething then
				unlockedSomething = true
				firstTime = true
			end
		end
	end

	return { names = correctNames, popup = firstTime }
end

-- File used to store progress
local config = "powderDinn.txt"

local saveProgress = function ()
	local file = io.open(config, "w")

	file:write("FACTOR " .. cipherFactor .. " 0\n")

	if userUnlockedVisible then
		file:write("ALMOST 0 0\n") -- Use useless 0s for the ease of parsing
	end

	for t = 1, ptMax do
		if elemExists[t] then
			local unlocked = unlockedElements[t] and "1" or "0"
			file:write("UNLOCK " .. t .. " " .. unlocked .. "\n")
		end
	end
	for t, v in pairs(guesses) do
		file:write("GUESS " .. t .. " " .. v .. "\n")

		if savedNotes[t] then
			file:write("NOTE " .. t .. " " .. savedNotes[t] .. "\n")
		end
	end

	for t, v in pairs(savedNotes) do
		if savedNotes[t] and not guesses[t] and not unlockedElements[t] then
			file:write("NOTE " .. t .. " " .. savedNotes[t] .. "\n")
		end
	end
	file:close()
end

-- Read the config file or create it if there isn't one
if fs.exists(config) then
	local file = io.open(config, "r")

	-- Read the file line by line
	for line in file:lines() do
		local words = line:gmatch("%S+")
		local configType = words()
		local t = tonumber(words())
		local arg3 = words()

		if configType == "FACTOR" then
			cipherFactor = t
		elseif configType == "ALMOST" then
			userUnlockedVisible = true
		elseif configType == "GUESS" then
			guesses[t] = arg3
		elseif configType == "UNLOCK" then
			if elemExists[t] and arg3 == "1" then
				unlockedElements[t] = true
				-- Prevent LIFE from being counted as "unlocked something"
				if t ~= elements.DEFAULT_PT_LIFE then
					unlockedSomething = true
				end
			end
		elseif configType == "NOTE" then
			local descr = arg3
			for wrd in words do
				descr = descr .. " " .. wrd
			end
			savedNotes[t] = descr
		end
	end

	-- Bad guesses in the config somehow
	for t, v in pairs(guesses) do
		if unlockedElements[t] then
			guesses[t] = nil
		end
	end

	file:close()
else
	interface.beginMessageBox("파우더 딘 호의 귀환",
	"당신은 제작자들조차 외면한, 신이 버린 이 비참한 게임으로 돌아왔다.\
    요소들의 이름은 세월 속에 희미해졌지만, 당신은 마치 어제 일처럼 그 이름을 기억하는 듯하다...\n\
    남아 있는 것을 구해낼 수 있을까? 모든 것을 원래의 모습으로 되돌릴 수 있을까?\n\
    장부가 기다리고 있다. 죽은 요소들이 다시금 제 목소리를 내도록 하라.")

	-- Setup cipherFactor
	local coprimeNumbers = {}
	local coprimeAmount = 0

	-- Greatest common divisor of two numbers
	local gcd = function (a, b)
		while b ~= 0 do
			a, b = b, a % b
		end
		return a
	end

	-- Compute numbers coprime to ptMax+1
	for t = 2, ptMax do
		if gcd(t, ptMax+1) == 1 then
			coprimeAmount = coprimeAmount + 1
			coprimeNumbers[coprimeAmount] = t
		end
	end

	-- Select a random cipherFactor
	if coprimeAmount > 0 then
		cipherFactor = coprimeNumbers[math.random(coprimeAmount)]
	end
end

-- Save progress on startup no matter what
saveProgress()

-- Scramble the not yet unlocked elements
scramble()

-- Save progress when exiting
event.register(event.close, saveProgress)

local function pos(component)
	local x, y = component:position()
	return {
		x = x,
		y = y
	}
end

local function size(component)
	local w, h = component:size()
	return {
		x = w,
		y = h
	}
end

local function positionFrom(component, offsetX, offsetY)
	local posX, posY = component:position()
	local sizeX, sizeY = component:size()
	return {
		x = posX + sizeX + offsetX,
		y = posY + sizeY + offsetY
	}
end

local function getLabel(x, y, text)
	local w, h = gfx.textSize(text)
	local label = Label:new(x - 1, y, w + 2, h - (h % 12) + 16, text)

	return label
end

local function drawElem(t, x, y, selected)
	local name = elem.property(t, "Name")
	local col = elem.property(t, "Color")
	local colr, colg, colb = gfx.getColors(col)
	gfx.fillRect(x + 2, y + 2, 26, 14, colr, colg, colb)
	local tcol = colb + colg * 3 + colr * 2 < 544 and 255 or 0
	gfx.drawText(x + 3 + (26 - gfx.textSize(name)) / 2, y + 5, name, tcol, tcol, tcol)

	-- Force cutoff long text
	gfx.fillRect(x, y, 2, 18, 0, 0, 0)
	gfx.fillRect(x + 28, y, 2, 18, 0, 0, 0)

	if selected then
		gfx.drawRect(x, y, 30, 18, 255, 0, 0)
	end
end

local function showProgressWindow()
	local progWindow = Window:new(-1, -1, 350, 250)
	progWindow:onTryExit(function() interface.closeWindow(progWindow) end)

	local titleLabel = getLabel(4, 2, "\x0F\x8C\x8C\xFF진행 상황 보고서")
	progWindow:addComponent(titleLabel)

	progWindow:onDraw(function()
		local titleLabelY = pos(progWindow).y + pos(titleLabel).y + size(titleLabel).y
		gfx.drawLine(pos(progWindow).x, titleLabelY, pos(progWindow).x + size(progWindow).x - 1, titleLabelY)
	end)

	local paginatedComponents = {}
	local pageNum = 1

	local function addProgress()
		local stillHidden

		if amountMenuHidden == 0 then
			stillHidden = getLabel(4, 20, "아직 \bo" .. amountHidden .. "\bw개의 요소가 구석구석에 숨어 있습니다.")
		else
			stillHidden = getLabel(4, 20, "아직 장부에 \bo" .. amountHidden .. "\bw개의 요소가 미해결 상태로 남아 있습니다.")
		end

		progWindow:addComponent(stillHidden)

		local infoLabel = getLabel(4, 50, "요소들의 원래 이름을 알아내십시오.\n정답을 세 번 연속으로 맞혀야만 진실이 밝혀집니다.\n\n현재 진행 중인 추론은 다음과 같습니다:")
		progWindow:addComponent(infoLabel)

		table.insert(paginatedComponents, stillHidden)
		table.insert(paginatedComponents, infoLabel)
	end


	local function addHeader(yPos)
		local elemLabel = getLabel(4, yPos, "이름")
		progWindow:addComponent(elemLabel)

		local descLabel = getLabel(40, yPos, "비고")
		progWindow:addComponent(descLabel)

		table.insert(paginatedComponents, elemLabel)
		table.insert(paginatedComponents, descLabel)
	end

	local function addGuess(yPos, name, description)
		local elemLabel = getLabel(4, yPos, name)
		progWindow:addComponent(elemLabel)

		local descLabel = getLabel(40, yPos, description)
		-- Description too big to fit. Cut it off to window width
		-- Trying to multiline it is almost entirely infeasible with the current api
		if size(descLabel).x > size(progWindow).x - 40 then
			descLabel = Label:new(39, yPos, size(progWindow).x - 40, 16, description)
		end
		progWindow:addComponent(descLabel)

		table.insert(paginatedComponents, elemLabel)
		table.insert(paginatedComponents, descLabel)
	end

	local function addAllGuesses(yPos)
		addHeader(yPos)
		yPos = yPos + 20
		for t, v in pairs(guesses) do
			addGuess(yPos, "\bt" .. v, elem.property(t, "Description"))
			yPos = yPos + 20
		end
		for t, v in pairs(savedNotes) do
			if not unlockedElements[t] and not guesses[t] then
				addGuess(yPos, elem.property(t, "Name"), elem.property(t, "Description"))
				yPos = yPos + 20
			end
		end
	end

	local function showPage(newPageNum)
		local perPage = size(progWindow).y - 75
		local yPos = 20 - (pageNum  - 1) * perPage
		local offset = (pageNum - newPageNum) * perPage

		local prevEnabled, nextEnabled = false, false
		for i, v in ipairs(paginatedComponents) do
			local currPos = pos(v)
			v:position(currPos.x, currPos.y + offset)
			if currPos.y + offset < 20 then
				v:visible(false)
				prevEnabled = true
			elseif currPos.y + offset > 20 + perPage then
				v:visible(false)
				nextEnabled = true
			else
				v:visible(true)
			end
		end

		pageNum = newPageNum
		return prevEnabled, nextEnabled
	end

	local function addPagination()
		local prevButton = Button:new(5, size(progWindow).y - 36, 60, 16, "이전 페이지")
		progWindow:addComponent(prevButton)

		local nextButton = Button:new(size(progWindow).x - 65, size(progWindow).y - 36, 60, 16, "다음 페이지")
		progWindow:addComponent(nextButton)

		local function changePage(offset)
			prevEnabled, nextEnabled = showPage(pageNum + offset)
			prevButton:enabled(prevEnabled)
			nextButton:enabled(nextEnabled)
		end
		prevButton:action(function(sender) changePage(-1) end)
		nextButton:action(function(sender) changePage(1) end)

		-- Set initial pagination
		changePage(0)
	end

	addProgress()
	addAllGuesses(positionFrom(paginatedComponents[#paginatedComponents], 0, 5).y)
	addPagination()

	local okButton = Button:new(0, size(progWindow).y - 16, size(progWindow).x, 16, "확인")
	okButton:action(function(sender)
		interface.closeWindow(progWindow)
	end)
	progWindow:addComponent(okButton)

	interface.showWindow(progWindow)
end

-- Define it early to use in showGuessWindow
local removeGuessButton

local function showGuessWindow()
	-- Guess window
	local guessWindow =  Window:new(-1, -1, 230, 140)
	guessWindow:onTryExit(function() interface.closeWindow(guessWindow) end)

	local titleLabel = getLabel(4, 2, "\x0F\x8C\x8C\xFF장부 최신화")
	guessWindow:addComponent(titleLabel)

	local function addTextboxes(yPos, oldName, newName, newDesc)
		local renameLabel = getLabel(size(guessWindow).x / 2 - 50, yPos, "수정")
		guessWindow:addComponent(renameLabel)

		local oldNameTextbox = Textbox:new(positionFrom(renameLabel, 5, 0).x + 5, yPos, 40, 16, oldName)
		guessWindow:addComponent(oldNameTextbox)

		local nameLabel = getLabel(7, positionFrom(oldNameTextbox, 0, 5).y, "새 이름")
		guessWindow:addComponent(nameLabel)

		local descLabel = getLabel(size(guessWindow).x / 3, positionFrom(oldNameTextbox, 0, 5).y, "비고(선택 사항)")
		guessWindow:addComponent(descLabel)

		local newNameTextbox = Textbox:new(pos(nameLabel).x, positionFrom(nameLabel, 0, 1).y, 40, 16, newName, "????")
		guessWindow:addComponent(newNameTextbox)

		local newDescTextbox = Textbox:new(pos(descLabel).x, positionFrom(descLabel, 0, 1).y, size(guessWindow).x * 2 / 3 - 10, 16, newDesc, "????")
		guessWindow:addComponent(newDescTextbox)

		if newNameTextbox.focus then
            newNameTextbox:focus(true)
        end

		oldNameTextbox:onTextChanged(function(sender)
			local t = decipher[sender:text()]
			if t then
				local desc = elem.property(t, "Description")
				if desc == "???" then return end
				newDescTextbox:text(desc)
			else
				newDescTextbox:text("")
			end
		end)

		return oldNameTextbox, newNameTextbox, newDescTextbox, positionFrom(newNameTextbox, 0, 5).y
	end

	local oldNameTextbox, newNameTextbox, newDescTextbox, nextY
	do
		local selectedElem = elem[tpt.selectedl]
		local oldName = selectedElem and not unlockedElements[selectedElem] and elem.property(selectedElem, "Name") or ""
		local newDesc = selectedElem and not unlockedElements[selectedElem] and elem.property(selectedElem, "Description") or ""
		if newDesc == "???" then newDesc = "" end
		oldNameTextbox, newNameTextbox, newDescTextbox, nextY = addTextboxes(positionFrom(titleLabel, 0, 5).y, oldName, "", newDesc)
	end

	local ledgerButton = Button:new(size(guessWindow).x / 2 - 40, size(guessWindow).y - 46, 96, 16, "진행 상황 보고서")
	ledgerButton:action(function(sender)
		showProgressWindow()
	end)
	guessWindow:addComponent(ledgerButton)


	local cancelButton = Button:new(0, size(guessWindow).y - 16, size(guessWindow).x * 2 / 3, 16, "취소")
	cancelButton:action(function(sender)
		interface.closeWindow(guessWindow)
	end)
	guessWindow:addComponent(cancelButton)

	local function removeGuess(t, tName)
        local ciphertext = cipher(t)
        guesses[t] = nil
        elements.property(t, "Name", ciphertext)
        decipher[ciphertext] = t
        decipher[tName] = nil
	end

	-- Validate textbox input, and show error messages to the user
	local function validate(oldName, newName, newDesc, t, validateCallback)
		if #oldName  == 0 then
			interface.beginThrowError("장부에서 업데이트할 요소를 먼저 선택하십시오.")
			return
		end
		if elem.getByName(oldName) and unlockedElements[elem.getByName(oldName)] then
			interface.beginThrowError("이미 잠금 해제한 요소의 이름은 변경할 수 없습니다.")
			return
		end
		if elem.getByName(newName) and unlockedElements[elem.getByName(newName)] then
			interface.beginThrowError("이미 해당 요소를 잠금 해제하였습니다.")
			return
		end

		if t and #newName == 0 then
			local oldDesc = elem.property(t, "Description")
			if oldDesc ~= newDesc then
				elem.property(t, "Description", newDesc)
				savedNotes[t] = newDesc
				interface.closeWindow(guessWindow)
				saveProgress() -- autosave
				return
			end
		end
		if #newName == 0 then
			if t and guesses[t] and #newName == 0 then
				interface.beginConfirm("추측을 제거하시겠습니까?", oldName .. "에 대한 추측을 제거하시겠습니까?", function(confirmed)
					if confirmed then
						removeGuess(t, oldName)
						interface.closeWindow(guessWindow)
					end
				end)
			else
				interface.beginThrowError("계속 진행하려면 원래 이름을 추측하거나, 확실하지 않다면 비고란을 수정하세요.")
			end
			return
		end
		if not correctInfo[newName] then
			interface.beginThrowError("광범위한 검색을 진행했으나, " .. string.upper(newName) .. "(이)라는 이름의 요소에 대한 기록은 발견되지 않았습니다.")
			return
		end
		if decipher[newName] then
			interface.beginConfirm("추측을 대체하시겠습니까?", "이미 다른 요소를 " .. newName .. "이라고 추측하셨습니다. 추측을 초기화하고 다시 입력하시겠습니까?", function(confirmed)
				if confirmed then
					local replacedType = decipher[newName]
					removeGuess(replacedType, newName)

					validateCallback()
				end
			end)
			return
		end

		-- Allow changing description and renaming elements at the same time
		if t then
			local oldDesc = elem.property(t, "Description")
			if oldDesc ~= newDesc then
				elem.property(t, "Description", newDesc)
				savedNotes[t] = newDesc
			end
		end

		validateCallback()
	end

	local function onOK()
		local oldName = string.upper(oldNameTextbox:text())
		local newName = string.upper(newNameTextbox:text())
		local newDesc = newDescTextbox:text()
		if #newDesc == 0 then newDesc = "???" end
		local t = decipher[oldName]

		local function afterValidate()
			if t == nil or decipher[newName] ~= nil or correctInfo[newName] == nil then
				return
			end

			guesses[t] = newName
			elements.property(t, "Name", newName)
			decipher[oldName] = nil
			decipher[newName] = t

			local answer = checkElements()

			saveProgress() -- autosave

			interface.closeWindow(guessWindow)

			-- Show elements which were guessed correctly or a winning screen
			if answer.popup then
				interface.beginMessageBox("첫 승리", "내 생각이 맞았어. 역시 " .. answer.names:sub(1, -3) ..
				"였군.\n\n시스템의 녹슨 톱니바퀴가 더 이상 이 게임의 전성기 때처럼 쉽게 돌아가지 않을 것 같아. " ..
				"이제부터는 세 번 연속으로 정확히 맞혀야지만 장부가 그 비밀을 드러낼 것이다. 다시 일하러 가야겠군.")
			elseif answer.names ~= "" then
				-- Show one of the messages
				if amountHidden == 0 then
					interface.beginMessageBox("장부가 완성되었습니다.",
					"안개는 걷혔다. 모든 알갱이, 모든 가루, 모든 불꽃이 하나도 빠짐없이 기록되었다. 이제 이곳에서의 내 일은 끝났다.")
					removeGuessButton()
				else
					if amountMenuHidden == 0 and not userUnlockedVisible then
						interface.beginMessageBox("끝?",
						"주요 내역은 모두 정리되었지만, 장부가 왠지 텅 비어 있는 것만 같군. " ..
						"여전히 구석구석에 무언가 남아 있어... 아직 이름 붙이지 못한 무언가가.")
						userUnlockedVisible = true
					else
						interface.beginMessageBox("요소가 복원되었습니다.", answer.names:sub(1, -3) .. "을(를) 복구하였습니다.")
					end
				end
			end
		end

		validate(oldName, newName, newDesc, t, afterValidate)
	end

	local okButton = Button:new(size(guessWindow).x * 2 / 3 - 1, size(guessWindow).y - 16, size(guessWindow).x / 3 + 1, 16, "\x0F\x8C\x8C\xFF확인")
	okButton:action(onOK)
	guessWindow:addComponent(okButton)

	guessWindow:onDraw(function()
		local titleLabelY = pos(guessWindow).y + pos(titleLabel).y + size(titleLabel).y
		gfx.drawLine(pos(guessWindow).x, titleLabelY, pos(guessWindow).x + size(guessWindow).x - 1, titleLabelY)
	end)

	guessWindow:onKeyPress(function(key, scan, rep, shift, ctrl, alt)
		if scan == ui.SDL_SCANCODE_RETURN or scan == ui.SDL_SCANCODE_KP_ENTER then
			onOK()
		end

		-- Not being able to focus components bothered me enough that I added this api
        if newNameTextbox.focus then
            if scan == ui.SDL_SCANCODE_TAB and not ctrl and not shift and not alt then
                if oldNameTextbox:focus() then
                    newNameTextbox:focus(true)
                elseif newNameTextbox:focus() then
                    newDescTextbox:focus(true)
                else
                    oldNameTextbox:focus(true)
                end
            end
        end
	end)

	interface.showWindow(guessWindow)
end

local guessButtonEvents = {}
local function addGuessButton()
	local buttonClicked = false
	local buttonX, buttonY = sim.XRES / 4, 10
	local buttonW, buttonH = 50, 17

	local function withinButton(x, y)
		return x >= buttonX and x < buttonX + buttonW and y >= buttonY and y < buttonY + buttonH
	end

	local function tick()
		local insideButton = withinButton(interface.mousePosition())
		if not insideButton then
			buttonClicked = false
		end

		local buttCol = insideButton and 255 or 225
		gfx.drawRect(buttonX, buttonY, buttonW, buttonH, buttCol, buttCol, buttCol, 255)
		do
			local fillCol, fillAlpha = 0, 255
			if buttonClicked then
				fillCol = 255
			elseif insideButton then
				fillCol = 255
				fillAlpha = 25
			end
			gfx.fillRect(buttonX + 1, buttonY + 1, buttonW - 2, buttonH - 2, fillCol, fillCol, fillCol, fillAlpha)
		end
		local textW, textH = gfx.textSize("추측")
		local textCol = buttonClicked and 0 or 255
		gfx.drawText(buttonX + (buttonW - textW) / 2, buttonY + (buttonH - textH + 2) / 2, "추측", textCol, textCol, textCol, 255)
	end

	local function mousedown(x, y, button)
		if withinButton(x, y) then
			buttonClicked = true
			return false
		end
	end

	local function mouseup(x, y, button)
		if buttonClicked and withinButton(x, y) then
			showGuessWindow()
			buttonClicked = false
		end
	end

	local function registerEvent(func, eventType)
		guessButtonEvents[eventType] = evt.register(eventType, func)
	end
	registerEvent(tick, evt.TICK)
	registerEvent(mousedown, evt.MOUSEDOWN)
	registerEvent(mouseup, evt.MOUSEUP)
end

removeGuessButton = function()
	for k, v in pairs(guessButtonEvents) do
		evt.unregister(k, v)
	end
	guessButtonEvents = {}
end

-- Don't show the button if everything is solved
if amountHidden > 0 then
	addGuessButton()
end

-- Intro pic
-- Legend:
-- 0 1 2 3
--     # #
--   #   #
local introPic = {{0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{3,2,3,3,2,2,2,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,3,3,2,2,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,3,3,1,1,1,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,3,3,0,0,0,3,3,0,2,2,0,1,0,0,0,0,0,1,0,0,0,0},{0,0,3,3,2,2,2,1,3,3,3,1,2,3,1,0,3,1,0,3,3,2,1,3,3,3,3,0,1,3,2,3,1,0,3,3,3,3,1,0,0,0,0,0,3,3,0,0,0,3,3,0,3,3,0,3,3,3,3,1,0,3,3,3,3,1},{1,0,3,3,0,0,0,3,3,0,3,3,0,3,3,1,3,3,1,3,3,0,3,3,0,3,3,0,3,3,1,3,2,0,3,3,0,0,0,0,0,0,1,0,3,3,0,0,0,3,3,0,3,3,0,3,3,0,3,3,0,3,3,0,3,3},{2,3,3,2,0,0,1,2,3,3,3,2,0,0,2,3,3,2,3,3,2,1,3,2,3,3,3,1,2,3,1,1,1,1,3,2,0,0,0,0,0,0,2,3,3,2,3,3,3,2,0,1,3,3,1,3,3,0,2,3,1,3,3,0,2,3},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,0,0,0,0},{0,0,0,0,0,2,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,3,3,0,0,0,0,1,1,1,3,3,1,1,1,0,3,3,0,0,0},{0,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0,0,0,3,3,0,0,0,0,0,1,3,2,2,3,1,0,0,3,3,0,0,0},{0,0,0,1,3,2,2,2,2,3,1,0,0,0,0,1,2,2,2,2,1,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,0,1,1,3,3,0,0,0,0,0,2,3,1,1,3,2,0,0,3,3,2,2,2},{0,0,0,3,3,0,0,0,0,3,3,0,0,0,0,3,0,0,0,0,3,0,0,3,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,3,3,2,2,2,0,0,3,3,0,0,0,1,1,1,1,3,3,1,1,1,2,3,3,0,0,0},{0,0,0,0,2,3,3,2,2,2,0,0,0,0,0,0,2,2,2,2,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0,0,0,0,0,3,3,0,0,0,0,0,0,1,1,1,0,0,0,0,2,0,0,0,0},{1,1,1,1,1,3,3,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0,0,0,0,0,3,3,0,0,0,0,0,0,0,3,3,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,3,2,0,0,0,0,0,0,0,2,3,1,1,1,1,1,1,0,0,0}}

local colorFade = 500

-- Start fading intro pic
local beginFade = function ()
	colorFade = math.min(colorFade, 255)
end

-- Draw intro pic
local drawPic
drawPic = function ()
	local height = #introPic
	local width = #(introPic[1])
	local x = 70
	local y = 60
	local size = 7

	if colorFade > 0 then
		colorFade = colorFade - 2

		for i = 1, height do
			for j = 1, width do
				local c = introPic[i][j]
				local color = math.min(255, colorFade)

				if c == 1 then
					graphics.fillRect(x + size*j, y + 2*size*i + size, size, size, 255, 255, 255, color)
				elseif c == 2 then
					graphics.fillRect(x + size*j, y + 2*size*i, size, size, 255, 255, 255, color)
				elseif c == 3 then
					graphics.fillRect(x + size*j, y + 2*size*i, size, 2*size, 255, 255, 255, color)
				end
			end
		end
	end

	-- Self-destruct intro pic
	if colorFade <= 0 then
		event.unregister(event.aftersimdraw, drawPic)
		event.unregister(event.mousedown, beginFade)
	end
end

-- Show intro picture if the game is incomplete
if not (userUnlockedVisible and amountHidden == 0) then
	-- Hide vanilla intro text
	tpt.hud(0)
	tpt.hud(1)

	event.register(event.aftersimdraw, drawPic)
	event.register(event.mousedown, beginFade)
end

-- Hidden but enabled elements
--[[
DYST, 64
MORT, 77 -- bad
LIFE, 78
LOVE, 94 -- bad
FRZW, 101
BIZG, 104
BIZS, 105
PSTS, 112
EQVE, 116 -- bad
SPWN2, 117
SPWN, 118
SHD2, 120
SHD3, 121
SHD4, 122
LOLZ, 123 -- bad
BRAY, 127
EMBR, 147
VRSS, 175
VRSG, 176
RFGL, 184
--]]
