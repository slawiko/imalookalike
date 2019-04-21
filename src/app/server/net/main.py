from src.app.server.net.prediction.predictor import check_model_inited, init_model, get_embedding


def process_image(data):
    if not check_model_inited():
        init_model()

    embedding = get_embedding(data)

    return embedding
