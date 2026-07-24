extends Control
# Chess Fighter - Full one-shot: chess + fighter duel on capture
# Foundation + fighter integration

var board: ChessBoard
var selected_pos: Vector2i = Vector2i(-1,-1)
var legal_moves_for_selected: Array[ChessMove] = []

var tile_size: int = 70
var board_offset: Vector2i = Vector2i(50,50)

var board_buttons: Array = []
var piece_labels: Array = []

var current_turn: ChessPiece.PieceColor = ChessPiece.PieceColor.WHITE

# Fighter integration - per spec: capture triggers fighter arena
var fighter_game: FighterGame = null
var fighter_ui_visible: bool = false
var pending_capture_move: ChessMove = null

@onready var status_label: Label = $StatusLabel
@onready var board_container: Control = $BoardContainer
@onready var fighter_container: Control = $FighterContainer
@onready var fighter_status: Label = $FighterContainer/FighterStatus
@onready var f1_hp_bar: ProgressBar = $FighterContainer/F1HP
@onready var f2_hp_bar: ProgressBar = $FighterContainer/F2HP
@onready var f1_name_label: Label = $FighterContainer/F1Name
@onready var f2_name_label: Label = $FighterContainer/F2Name
@onready var f1_pos_label: Label = $FighterContainer/Fighter1Pos
@onready var f2_pos_label: Label = $FighterContainer/Fighter2Pos

func _ready():
	board = ChessBoard.new()
	_setup_board_ui()
	if fighter_container:
		fighter_container.visible = false
	fighter_ui_visible = false
	_update_status()
	print("Chess Fighter ready - capture triggers fighter duel!")

func _setup_board_ui():
	for child in board_container.get_children():
		child.queue_free()
	board_buttons = []
	piece_labels = []

	for y in range(8):
		var row_buttons = []
		var row_labels = []
		for x in range(8):
			var tile = ColorRect.new()
			tile.size = Vector2(tile_size, tile_size)
			tile.position = Vector2(board_offset.x + x*tile_size, board_offset.y + y*tile_size)
			var is_light = (x+y)%2==0
			tile.color = Color(0.9,0.9,0.8) if is_light else Color(0.4,0.6,0.4)
			board_container.add_child(tile)

			var btn = Button.new()
			btn.size = Vector2(tile_size, tile_size)
			btn.position = tile.position
			btn.flat = true
			btn.name = "TileBtn_%d_%d" % [x, y]
			var pos = Vector2i(x,y)
			btn.pressed.connect(_on_tile_pressed.bind(pos))
			board_container.add_child(btn)
			row_buttons.append(btn)

			var lbl = Label.new()
			lbl.size = Vector2(tile_size, tile_size)
			lbl.position = tile.position
			lbl.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
			lbl.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
			lbl.add_theme_font_size_override("font_size", 36)
			lbl.mouse_filter = Control.MOUSE_FILTER_IGNORE
			board_container.add_child(lbl)
			row_labels.append(lbl)
		board_buttons.append(row_buttons)
		piece_labels.append(row_labels)
	_refresh_board_visuals()

func _refresh_board_visuals():
	for y in range(8):
		for x in range(8):
			var pos = Vector2i(x,y)
			var piece = board.get_piece(pos)
			var lbl: Label = piece_labels[y][x]
			if piece == null:
				lbl.text = ""
			else:
				lbl.text = piece.get_unicode_char()
				lbl.add_theme_color_override("font_color", Color.WHITE if piece.color==ChessPiece.PieceColor.WHITE else Color.BLACK)
				if piece.color == ChessPiece.PieceColor.WHITE:
					lbl.add_theme_color_override("font_outline_color", Color.BLACK)
					lbl.add_theme_constant_override("outline_size", 3)
				else:
					lbl.add_theme_constant_override("outline_size", 0)
			var btn: Button = board_buttons[y][x]
			if pos == selected_pos:
				btn.modulate = Color(1,1,0.5)
			else:
				var is_legal = false
				for m in legal_moves_for_selected:
					if m.to == pos:
						is_legal = true
						break
				if is_legal:
					btn.modulate = Color(0.6,1,0.6) if board.get_piece(pos)==null else Color(1,0.5,0.5)
				else:
					btn.modulate = Color(1,1,1)

func _on_tile_pressed(pos: Vector2i):
	if fighter_ui_visible:
		return # block chess input during fighter
	if board.is_checkmate(current_turn) or board.is_stalemate(current_turn):
		return
	var piece = board.get_piece(pos)
	if selected_pos == Vector2i(-1,-1):
		if piece != null and piece.color == current_turn:
			selected_pos = pos
			var all_legal = board.generate_legal_moves(current_turn)
			legal_moves_for_selected = []
			for m in all_legal:
				if m.from == pos:
					legal_moves_for_selected.append(m)
			_refresh_board_visuals()
	else:
		if pos == selected_pos:
			selected_pos = Vector2i(-1,-1)
			legal_moves_for_selected = []
			_refresh_board_visuals()
			return
		if piece != null and piece.color == current_turn:
			selected_pos = pos
			var all_legal = board.generate_legal_moves(current_turn)
			legal_moves_for_selected = []
			for m in all_legal:
				if m.from == pos:
					legal_moves_for_selected.append(m)
			_refresh_board_visuals()
			return
		# Try move
		var chosen: ChessMove = null
		for m in legal_moves_for_selected:
			if m.to == pos:
				chosen = m
				break
		if chosen != null:
			# Capture? -> trigger fighter per one-shot spec
			if chosen.captured_piece != null:
				print("Capture attempt %s -> triggering fighter duel!" % chosen.to_algebraic())
				_trigger_fighter(chosen)
			else:
				board.apply_move(chosen)
				print("Move: %s" % chosen.to_algebraic())
				selected_pos = Vector2i(-1,-1)
				legal_moves_for_selected = []
				current_turn = ChessPiece.PieceColor.BLACK if current_turn==ChessPiece.PieceColor.WHITE else ChessPiece.PieceColor.WHITE
				_refresh_board_visuals()
				_update_status()
		else:
			var from_before = selected_pos
			print("Invalid move attempted from %s to %s" % [from_before, pos])
			selected_pos = Vector2i(-1,-1)
			legal_moves_for_selected = []
			_refresh_board_visuals()
			if status_label:
				status_label.text = "Invalid move! %s -> %s not legal. Pick another piece." % [from_before, pos]
				get_tree().create_timer(1.5).timeout.connect(func(): _update_status())

func _trigger_fighter(move: ChessMove):
	var attacker = board.get_piece(move.from)
	var defender = board.get_piece(move.to)
	if attacker == null or defender == null:
		# Fallback if captured_piece reference used
		defender = move.captured_piece
	if attacker == null or defender == null:
		print("ERROR: attacker/defender null for fighter")
		return

	pending_capture_move = move
	fighter_game = FighterGame.new(attacker, defender)
	fighter_game.fighter_hit.connect(_on_fighter_hit)
	fighter_game.fighter_ko.connect(_on_fighter_ko)
	fighter_game.fighter_special_used.connect(_on_special_used)

	fighter_ui_visible = true
	if fighter_container:
		fighter_container.visible = true
	_update_fighter_ui()

	if status_label:
		status_label.text = "FIGHT! %s (%s) vs %s (%s) — A/D move, J/K/U/I attack, Shift block, Space special" % [
			attacker.get_symbol(), attacker.get_fighter_special_name(),
			defender.get_symbol(), defender.get_fighter_special_name()
		]
	print("Fighter started: %s vs %s" % [attacker.get_fighter_special_name(), defender.get_fighter_special_name()])

func _process(delta):
	if fighter_ui_visible and fighter_game != null:
		var finished = fighter_game.update(delta)
		_update_fighter_ui()
		# AI for defender
		if not finished:
			fighter_game.try_fighter2_ai_move()
		else:
			# Fighter finished
			var attacker_won = fighter_game.attacker_won()
			fighter_ui_visible = false
			if fighter_container:
				fighter_container.visible = false

			if attacker_won:
				print("Attacker won fighter - capture succeeds!")
				if pending_capture_move:
					board.apply_move(pending_capture_move)
					selected_pos = Vector2i(-1,-1)
					legal_moves_for_selected = []
					current_turn = ChessPiece.PieceColor.BLACK if current_turn==ChessPiece.PieceColor.WHITE else ChessPiece.PieceColor.WHITE
					if status_label:
						status_label.text = "Attacker won duel! Capture succeeds!"
			else:
				print("Defender won fighter - capture denied!")
				selected_pos = Vector2i(-1,-1)
				legal_moves_for_selected = []
				current_turn = ChessPiece.PieceColor.BLACK if current_turn==ChessPiece.PieceColor.WHITE else ChessPiece.PieceColor.WHITE
				if status_label:
					status_label.text = "Defender won duel! Capture denied!"

			fighter_game = null
			pending_capture_move = null
			_refresh_board_visuals()
			_update_status()

			# Small delay before next turn status normalizes
			get_tree().create_timer(2.0).timeout.connect(func(): _update_status())

func _update_status():
	if board == null:
		return
	if fighter_ui_visible:
		return # don't overwrite fighter status
	var t = "White" if current_turn==ChessPiece.PieceColor.WHITE else "Black"
	var check = board.is_check(current_turn)
	var mate = board.is_checkmate(current_turn)
	var stale = board.is_stalemate(current_turn)
	var msg = "%s to move | " % t
	if mate:
		msg += "CHECKMATE! %s wins!" % ("Black" if current_turn==ChessPiece.PieceColor.WHITE else "White")
	elif stale:
		msg += "STALEMATE - Draw!"
	elif check:
		msg += "CHECK!"
	else:
		msg += "Click piece then destination. Captures trigger Fighter!"
	if status_label:
		status_label.text = msg

func _update_fighter_ui():
	if fighter_game == null:
		return
	if not fighter_container:
		return
	var f1 = fighter_game.fighter1
	var f2 = fighter_game.fighter2
	if f1_hp_bar:
		f1_hp_bar.value = f1.get_hp_percent() * 100
	if f2_hp_bar:
		f2_hp_bar.value = f2.get_hp_percent() * 100
	if f1_name_label:
		f1_name_label.text = "%s HP:%d" % [f1.get_display_name(), f1.hp]
	if f2_name_label:
		f2_name_label.text = "%s HP:%d" % [f2.get_display_name(), f2.hp]
	if fighter_status:
		fighter_status.text = "FIGHTER: %s vs %s | %s / %s | Time %.1f | Attacker must win!" % [
			f1.get_display_name(), f2.get_display_name(),
			f1.get_special_name(), f2.get_special_name(),
			fighter_game.get_time_remaining()
		]
	if f1_pos_label:
		f1_pos_label.text = "%s @ %d" % [f1.get_display_name(), int(f1.position_x)]
	if f2_pos_label:
		f2_pos_label.text = "%s @ %d" % [f2.get_display_name(), int(f2.position_x)]

func _on_fighter_hit(attacker: FighterCharacter, defender: FighterCharacter, damage: int, move_type: String):
	print("Fighter hit: %s -> %s %d dmg %s" % [attacker.get_display_name(), defender.get_display_name(), damage, move_type])

func _on_fighter_ko(winner: FighterCharacter, loser: FighterCharacter):
	print("Fighter KO: %s defeated %s" % [winner.get_display_name(), loser.get_display_name()])

func _on_special_used(user: FighterCharacter, name: String):
	print("Special! %s used %s" % [user.get_display_name(), name])

func _unhandled_input(event):
	if not fighter_ui_visible or fighter_game == null:
		return
	if event is InputEventKey and event.pressed:
		match event.physical_keycode:
			KEY_J:
				fighter_game.try_fighter1_move("punch_light")
			KEY_K:
				fighter_game.try_fighter1_move("punch_heavy")
			KEY_U:
				fighter_game.try_fighter1_move("kick_light")
			KEY_I:
				fighter_game.try_fighter1_move("kick_heavy")
			KEY_SHIFT:
				fighter_game.try_fighter1_move("block")
			KEY_SPACE:
				fighter_game.try_fighter1_move("special")
			KEY_A:
				fighter_game.fighter1.position_x -= 15
				fighter_game.fighter1.position_x = clamp(fighter_game.fighter1.position_x, 50.0, fighter_game.arena_width - 50.0)
			KEY_D:
				fighter_game.fighter1.position_x += 15
				fighter_game.fighter1.position_x = clamp(fighter_game.fighter1.position_x, 50.0, fighter_game.arena_width - 50.0)
