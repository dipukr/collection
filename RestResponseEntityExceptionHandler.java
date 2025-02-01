package in.hertz.samast.errorhandling;

import java.util.HashMap;
import java.util.Map;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.validation.FieldError;
import org.springframework.web.bind.MethodArgumentNotValidException;
import org.springframework.web.bind.annotation.ControllerAdvice;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.context.request.ServletWebRequest;
import org.springframework.web.context.request.WebRequest;
import org.springframework.web.servlet.mvc.method.annotation.ResponseEntityExceptionHandler;

import in.hertz.samast.domain.WSResp;
import in.hertz.samast.domain.exception.BusinessException;

@ControllerAdvice
public class RestResponseEntityExceptionHandler extends ResponseEntityExceptionHandler {

	private static final Logger LOG = LogManager.getLogger(RestResponseEntityExceptionHandler.class);

	@ExceptionHandler(value = { BusinessException.class, Exception.class })
	protected ResponseEntity<Object> handleConflict(Exception ex, ServletWebRequest request) {

		Throwable cause = ex.getCause();
		WSResp<String> bodyOfResponse = null;
		HttpStatus httpStatus = null;
		if (cause instanceof BusinessException) {
			bodyOfResponse = new WSResp<>(cause.getMessage());
			httpStatus = HttpStatus.INTERNAL_SERVER_ERROR;
			LOG.error(request, cause);
		} else {
			bodyOfResponse = new WSResp<>(ex.getMessage());
			httpStatus = HttpStatus.INTERNAL_SERVER_ERROR;
			LOG.error(request, ex);
		}

		return handleExceptionInternal(ex, bodyOfResponse, new HttpHeaders(), httpStatus, request);
	}
	@Override
	protected ResponseEntity<Object> handleMethodArgumentNotValid(MethodArgumentNotValidException ex,
			HttpHeaders headers, HttpStatus status, WebRequest request) {
		
		Map<String, String> errors = new HashMap<>();
		ex.getBindingResult().getAllErrors().forEach((error) ->{
			
			
			String fieldName = ((FieldError) error).getField();
			String message = error.getDefaultMessage();
			errors.put(fieldName, message);
		});
		return new ResponseEntity<Object>(errors, HttpStatus.INTERNAL_SERVER_ERROR);
	}
}
