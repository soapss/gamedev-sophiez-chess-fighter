extends Control
# Step 1: Just build out the chess board first — no fighter yet
# This is the gold foundation for Chess Fighter one-shot

var board: ChessBoard
var selected_pos: Vector2i = Vector2i(-1,-1)
var legal_moves_for_selected: Array[ChessMove] = []

var tile_size: int = 70
var board_offset: Vector2i = Vector2i(50,50)

var board_buttons: Array = [] # [y][x] buttons
var piece_labels: Array = [] # [y][x] labels

var current_turn: ChessPiece.PieceColor = ChessPiece.PieceColor.WHITE

@onready var status_label: Label = $StatusLabel
@onready var board_container: Control = $BoardContainer

func _ready():
	board = ChessBoard.new()
	_setup_board_ui()
	_update_status()

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
			# Use bind() to avoid closure capture bug in Godot 4 that made all buttons point to (7,7)
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
			board.apply_move(chosen)
			print("Move: %s" % chosen.to_algebraic())
			selected_pos = Vector2i(-1,-1)
			legal_moves_for_selected = []
			# Switch turn
			current_turn = ChessPiece.PieceColor.BLACK if current_turn==ChessPiece.PieceColor.WHITE else ChessPiece.PieceColor.WHITE
			_refresh_board_visuals()
			_update_status()
		else:
			# invalid — was previously freezing UX because selection stayed stuck with no refresh
			var from_before = selected_pos
			print("Invalid move attempted from %s to %s" % [from_before, pos])
			# Clear selection so board doesn't feel frozen - user can pick new piece immediately
			selected_pos = Vector2i(-1,-1)
			legal_moves_for_selected = []
			_refresh_board_visuals()
			if status_label:
				status_label.text = "Invalid move! %s -> %s not legal. Pick another piece." % [from_before, pos]
				# Restore normal status after 1.5s so it doesn't stay stuck
				# Use timer without blocking input
				get_tree().create_timer(1.5).timeout.connect(func(): _update_status())

func _update_status():
	if board == null:
		return
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
		msg += "Click piece then destination"
	status_label.text = msg
