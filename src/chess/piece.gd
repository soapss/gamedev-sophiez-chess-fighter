class_name ChessPiece

enum Type { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING }
enum PieceColor { WHITE, BLACK }

var type: Type
var color: PieceColor
var has_moved: bool = false

func _init(p_type: Type = Type.PAWN, p_color: PieceColor = PieceColor.WHITE):
	type = p_type
	color = p_color

func get_symbol() -> String:
	match type:
		Type.PAWN: return "P"
		Type.KNIGHT: return "N"
		Type.BISHOP: return "B"
		Type.ROOK: return "R"
		Type.QUEEN: return "Q"
		Type.KING: return "K"
	return "?"

func get_unicode_char() -> String:
	if color == PieceColor.WHITE:
		match type:
			Type.KING: return "♔"
			Type.QUEEN: return "♕"
			Type.ROOK: return "♖"
			Type.BISHOP: return "♗"
			Type.KNIGHT: return "♘"
			Type.PAWN: return "♙"
	else:
		match type:
			Type.KING: return "♚"
			Type.QUEEN: return "♛"
			Type.ROOK: return "♜"
			Type.BISHOP: return "♝"
			Type.KNIGHT: return "♞"
			Type.PAWN: return "♟"
	return "?"

func get_material_value() -> int:
	match type:
		Type.PAWN: return 100
		Type.KNIGHT: return 320
		Type.BISHOP: return 330
		Type.ROOK: return 500
		Type.QUEEN: return 900
		Type.KING: return 20000
	return 0

func get_fighter_special_name() -> String:
	match type:
		Type.PAWN: return "Pawn Rush"
		Type.KNIGHT: return "Knight Leap"
		Type.BISHOP: return "Bishop Warp"
		Type.ROOK: return "Rook Charge"
		Type.QUEEN: return "Queen's Wrath"
		Type.KING: return "Royal Shield"
	return "Unknown"

func get_fighter_hp() -> int:
	match type:
		Type.PAWN: return 80
		Type.KNIGHT: return 100
		Type.BISHOP: return 95
		Type.ROOK: return 120
		Type.QUEEN: return 130
		Type.KING: return 110
	return 100

func duplicate_piece() -> ChessPiece:
	var p = ChessPiece.new(type, color)
	p.has_moved = has_moved
	return p
