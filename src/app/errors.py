import flask


class InvalidImage(Exception):
    def __init__(self, message='Invalid image data', status_code=415):
        self.message = message
        self.status_code = status_code

    def to_response(self):
        return flask.make_response(self.message, self.status_code)

