class_name ChessBoard

# 8x8 grid, y=0 is rank 8 (black back rank), y=7 is rank 1 (white back rank)
var grid: Array = [] # [y][x] -> ChessPiece or null

var en_passant_target: Vector2i = Vector2i(-1,-1)
var white_to_move: bool = true

func _init():
	_init_empty()
	setup_initial()

func _init_empty():
	grid = []
	for y in range(8):
		var row = []
		for x in range(8):
			row.append(null)
		grid.append(row)
	en_passant_target = Vector2i(-1,-1)

func setup_initial():
	_init_empty()
	# Pawns
	for x in range(8):
		grid[1][x] = ChessPiece.new(ChessPiece.Type.PAWN, ChessPiece.Color.BLACK)
		grid[6][x] = ChessPiece.new(ChessPiece.Type.PAWN, ChessPiece.Color.WHITE)
	# Rooks
	grid[0][0] = ChessPiece.new(ChessPiece.Type.ROOK, ChessPiece.Color.BLACK)
	grid[0][7] = ChessPiece.new(ChessPiece.Type.ROOK, ChessPiece.Color.BLACK)
	grid[7][0] = ChessPiece.new(ChessPiece.Type.ROOK, ChessPiece.Color.WHITE)
	grid[7][7] = ChessPiece.new(ChessPiece.Type.ROOK, ChessPiece.Color.WHITE)
	# Knights
	grid[0][1] = ChessPiece.new(ChessPiece.Type.KNIGHT, ChessPiece.Color.BLACK)
	grid[0][6] = ChessPiece.new(ChessPiece.Type.KNIGHT, ChessPiece.Color.BLACK)
	grid[7][1] = ChessPiece.new(ChessPiece.Type.KNIGHT, ChessPiece.Color.WHITE)
	grid[7][6] = ChessPiece.new(ChessPiece.Type.KNIGHT, ChessPiece.Color.WHITE)
	# Bishops
	grid[0][2] = ChessPiece.new(ChessPiece.Type.BISHOP, ChessPiece.Color.BLACK)
	grid[0][5] = ChessPiece.new(ChessPiece.Type.BISHOP, ChessPiece.Color.BLACK)
	grid[7][2] = ChessPiece.new(ChessPiece.Type.BISHOP, ChessPiece.Color.WHITE)
	grid[7][5] = ChessPiece.new(ChessPiece.Type.BISHOP, ChessPiece.Color.WHITE)
	# Queens
	grid[0][3] = ChessPiece.new(ChessPiece.Type.QUEEN, ChessPiece.Color.BLACK)
	grid[7][3] = ChessPiece.new(ChessPiece.Type.QUEEN, ChessPiece.Color.WHITE)
	# Kings
	grid[0][4] = ChessPiece.new(ChessPiece.Type.KING, ChessPiece.Color.BLACK)
	grid[7][4] = ChessPiece.new(ChessPiece.Type.KING, ChessPiece.Color.WHITE)

func get_piece(pos: Vector2i) -> ChessPiece:
	if not _in_bounds(pos):
		return null
	return grid[pos.y][pos.x]

func set_piece(pos: Vector2i, piece: ChessPiece):
	if _in_bounds(pos):
		grid[pos.y][pos.x] = piece

func _in_bounds(pos: Vector2i) -> bool:
	return pos.x >=0 and pos.x <8 and pos.y >=0 and pos.y <8

func duplicate() -> ChessBoard:
	var nb = ChessBoard.new()
	nb._init_empty()
	for y in range(8):
		for x in range(8):
			var p = grid[y][x]
			if p != null:
				nb.grid[y][x] = p.duplicate_piece()
			else:
				nb.grid[y][x] = null
	nb.en_passant_target = en_passant_target
	nb.white_to_move = white_to_move
	return nb

func find_king(color: ChessPiece.Color) -> Vector2i:
	for y in range(8):
		for x in range(8):
			var p = grid[y][x]
			if p != null and p.type == ChessPiece.Type.KING and p.color == color:
				return Vector2i(x,y)
	return Vector2i(-1,-1)

func is_path_clear(from: Vector2i, to: Vector2i) -> bool:
	var dx = sign(to.x - from.x)
	var dy = sign(to.y - from.y)
	var cur = from + Vector2i(dx,dy)
	while cur != to:
		if get_piece(cur) != null:
			return false
		cur += Vector2i(dx,dy)
	return true

func _piece_attacks_square(piece_pos: Vector2i, target: Vector2i) -> bool:
	var piece = get_piece(piece_pos)
	if piece == null:
		return false
	var dx = target.x - piece_pos.x
	var dy = target.y - piece_pos.y

	match piece.type:
		ChessPiece.Type.PAWN:
			var dir = -1 if piece.color == ChessPiece.Color.WHITE else 1
			if dy == dir and abs(dx) == 1:
				return true
			return false
		ChessPiece.Type.KNIGHT:
			return (abs(dx)==1 and abs(dy)==2) or (abs(dx)==2 and abs(dy)==1)
		ChessPiece.Type.BISHOP:
			if abs(dx) != abs(dy):
				return false
			return is_path_clear(piece_pos, target)
		ChessPiece.Type.ROOK:
			if dx !=0 and dy !=0:
				return false
			return is_path_clear(piece_pos, target)
		ChessPiece.Type.QUEEN:
			if abs(dx)==abs(dy) or dx==0 or dy==0:
				return is_path_clear(piece_pos, target)
			return false
		ChessPiece.Type.KING:
			return abs(dx) <=1 and abs(dy) <=1
	return false

func is_square_attacked(pos: Vector2i, by_color: ChessPiece.Color) -> bool:
	for y in range(8):
		for x in range(8):
			var p = grid[y][x]
			if p != null and p.color == by_color:
				if _piece_attacks_square(Vector2i(x,y), pos):
					return true
	return false

func generate_pseudo_legal_moves(color: ChessPiece.Color) -> Array[ChessMove]:
	var moves: Array[ChessMove] = []
	for y in range(8):
		for x in range(8):
			var piece = grid[y][x]
			if piece == null or piece.color != color:
				continue
			var from = Vector2i(x,y)
			moves.append_array(_gen_piece_moves(from, piece))
	return moves

func _gen_piece_moves(from: Vector2i, piece: ChessPiece) -> Array[ChessMove]:
	var moves: Array[ChessMove] = []
	var x = from.x
	var y = from.y

	match piece.type:
		ChessPiece.Type.PAWN:
			var dir = -1 if piece.color == ChessPiece.Color.WHITE else 1
			var start_rank = 6 if piece.color == ChessPiece.Color.WHITE else 1

			# forward 1
			var one = Vector2i(x, y + dir)
			if _in_bounds(one) and get_piece(one) == null:
				if one.y == 0 or one.y == 7:
					var m = ChessMove.new(from, one)
					m.is_promotion = true
					moves.append(m)
				else:
					moves.append(ChessMove.new(from, one))
				# forward 2 from start
				if y == start_rank:
					var two = Vector2i(x, y + dir*2)
					if _in_bounds(two) and get_piece(two) == null:
						moves.append(ChessMove.new(from, two))

			# captures
			for dx in [-1,1]:
				var cap = Vector2i(x+dx, y+dir)
				if not _in_bounds(cap):
					continue
				var target = get_piece(cap)
				if target != null and target.color != piece.color:
					if cap.y ==0 or cap.y==7:
						var m = ChessMove.new(from, cap)
						m.is_promotion = true
						m.captured_piece = target
						moves.append(m)
					else:
						var m = ChessMove.new(from, cap)
						m.captured_piece = target
						moves.append(m)
				# en passant
				if cap == en_passant_target:
					var m = ChessMove.new(from, cap)
					m.is_en_passant = true
					# captured pawn is behind
					var captured_pos = Vector2i(cap.x, cap.y - dir)
					m.captured_piece = get_piece(captured_pos)
					moves.append(m)

		ChessPiece.Type.KNIGHT:
			var offsets = [Vector2i(1,2), Vector2i(2,1), Vector2i(-1,2), Vector2i(-2,1), Vector2i(1,-2), Vector2i(2,-1), Vector2i(-1,-2), Vector2i(-2,-1)]
			for off in offsets:
				var to = from + off
				if not _in_bounds(to):
					continue
				var t = get_piece(to)
				if t == null or t.color != piece.color:
					var m = ChessMove.new(from, to)
					m.captured_piece = t
					moves.append(m)

		ChessPiece.Type.BISHOP, ChessPiece.Type.ROOK, ChessPiece.Type.QUEEN:
			var dirs: Array[Vector2i] = []
			if piece.type == ChessPiece.Type.BISHOP or piece.type == ChessPiece.Type.QUEEN:
				dirs.append_array([Vector2i(1,1), Vector2i(-1,1), Vector2i(1,-1), Vector2i(-1,-1)])
			if piece.type == ChessPiece.Type.ROOK or piece.type == ChessPiece.Type.QUEEN:
				dirs.append_array([Vector2i(1,0), Vector2i(-1,0), Vector2i(0,1), Vector2i(0,-1)])
			for d in dirs:
				var cur = from + d
				while _in_bounds(cur):
					var t = get_piece(cur)
					if t == null:
						moves.append(ChessMove.new(from, cur))
					else:
						if t.color != piece.color:
							var m = ChessMove.new(from, cur)
							m.captured_piece = t
							moves.append(m)
						break
					cur += d

		ChessPiece.Type.KING:
			for dx in [-1,0,1]:
				for dy in [-1,0,1]:
					if dx==0 and dy==0:
						continue
					var to = Vector2i(x+dx, y+dy)
					if not _in_bounds(to):
						continue
					var t = get_piece(to)
					if t == null or t.color != piece.color:
						var m = ChessMove.new(from, to)
						m.captured_piece = t
						moves.append(m)
			# castling
			if not piece.has_moved:
				# kingside
				if _can_castle_kingside(piece.color):
					var to = Vector2i(x+2, y)
					var m = ChessMove.new(from, to)
					m.is_castling = true
					moves.append(m)
				if _can_castle_queenside(piece.color):
					var to = Vector2i(x-2, y)
					var m = ChessMove.new(from, to)
					m.is_castling = true
					moves.append(m)

	return moves

func _can_castle_kingside(color: ChessPiece.Color) -> bool:
	var rank = 7 if color == ChessPiece.Color.WHITE else 0
	var king_pos = Vector2i(4, rank)
	var king = get_piece(king_pos)
	if king == null or king.has_moved:
		return false
	var rook_pos = Vector2i(7, rank)
	var rook = get_piece(rook_pos)
	if rook == null or rook.type != ChessPiece.Type.ROOK or rook.has_moved:
		return false
	# squares between must be empty
	if get_piece(Vector2i(5,rank)) != null or get_piece(Vector2i(6,rank)) != null:
		return false
	# king not in check, and intermediate squares not attacked
	var opp = ChessPiece.Color.BLACK if color==ChessPiece.Color.WHITE else ChessPiece.Color.WHITE
	if is_square_attacked(king_pos, opp):
		return false
	if is_square_attacked(Vector2i(5,rank), opp) or is_square_attacked(Vector2i(6,rank), opp):
		return false
	return true

func _can_castle_queenside(color: ChessPiece.Color) -> bool:
	var rank = 7 if color == ChessPiece.Color.WHITE else 0
	var king_pos = Vector2i(4, rank)
	var king = get_piece(king_pos)
	if king == null or king.has_moved:
		return false
	var rook_pos = Vector2i(0, rank)
	var rook = get_piece(rook_pos)
	if rook == null or rook.type != ChessPiece.Type.ROOK or rook.has_moved:
		return false
	if get_piece(Vector2i(1,rank)) != null or get_piece(Vector2i(2,rank)) != null or get_piece(Vector2i(3,rank)) != null:
		return false
	var opp = ChessPiece.Color.BLACK if color==ChessPiece.Color.WHITE else ChessPiece.Color.WHITE
	if is_square_attacked(king_pos, opp):
		return false
	if is_square_attacked(Vector2i(3,rank), opp) or is_square_attacked(Vector2i(2,rank), opp):
		return false
	return true

func generate_legal_moves(color: ChessPiece.Color) -> Array[ChessMove]:
	var pseudo = generate_pseudo_legal_moves(color)
	var legal: Array[ChessMove] = []
	for m in pseudo:
		var copy = duplicate()
		copy.apply_move(m)
		var king_pos = copy.find_king(color)
		if king_pos.x == -1:
			continue
		var opp = ChessPiece.Color.BLACK if color==ChessPiece.Color.WHITE else ChessPiece.Color.WHITE
		if not copy.is_square_attacked(king_pos, opp):
			legal.append(m)
	return legal

func apply_move(move: ChessMove):
	var piece = get_piece(move.from)
	if piece == null:
		return

	# handle en passant capture
	if move.is_en_passant:
		var dir = -1 if piece.color == ChessPiece.Color.WHITE else 1
		# Actually en passant target is the square moved to, captured pawn is behind
		var captured_pos = Vector2i(move.to.x, move.to.y - dir)
		set_piece(captured_pos, null)

	# handle castling rook move
	if move.is_castling:
		var rank = move.from.y
		if move.to.x > move.from.x:
			# kingside
			var rook_from = Vector2i(7, rank)
			var rook_to = Vector2i(5, rank)
			var rook = get_piece(rook_from)
			set_piece(rook_to, rook)
			set_piece(rook_from, null)
			if rook != null:
				rook.has_moved = true
		else:
			# queenside
			var rook_from = Vector2i(0, rank)
			var rook_to = Vector2i(3, rank)
			var rook = get_piece(rook_from)
			set_piece(rook_to, rook)
			set_piece(rook_from, null)
			if rook != null:
				rook.has_moved = true

	# move piece
	set_piece(move.to, piece)
	set_piece(move.from, null)
	piece.has_moved = true

	# handle promotion
	if move.is_promotion:
		var promoted = ChessPiece.new(move.promotion, piece.color)
		promoted.has_moved = true
		set_piece(move.to, promoted)

	# set en passant target
	en_passant_target = Vector2i(-1,-1)
	if piece.type == ChessPiece.Type.PAWN and abs(move.to.y - move.from.y) == 2:
		en_passant_target = Vector2i(move.from.x, (move.from.y + move.to.y)/2)

func evaluate(color: ChessPiece.Color) -> int:
	var score = 0
	for y in range(8):
		for x in range(8):
			var p = grid[y][x]
			if p == null:
				continue
			var val = p.get_material_value()
			# small positional bonus for pawns advancing
			if p.type == ChessPiece.Type.PAWN:
				var bonus = (7 - y) if p.color == ChessPiece.Color.WHITE else y
				val += bonus * 5
			if p.color == ChessPiece.Color.WHITE:
				score += val
			else:
				score -= val
	if color == ChessPiece.Color.WHITE:
		return score
	else:
		return -score

func is_check(color: ChessPiece.Color) -> bool:
	var kpos = find_king(color)
	if kpos.x == -1:
		return false
	var opp = ChessPiece.Color.BLACK if color==ChessPiece.Color.WHITE else ChessPiece.Color.WHITE
	return is_square_attacked(kpos, opp)

func is_checkmate(color: ChessPiece.Color) -> bool:
	if not is_check(color):
		return false
	return generate_legal_moves(color).size() == 0

func is_stalemate(color: ChessPiece.Color) -> bool:
	if is_check(color):
		return false
	return generate_legal_moves(color).size() == 0
